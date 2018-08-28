/* m_nickcaps.cpp
 *
 * InspIRCd 2.0 branch module that adds a channel mode (+U) to prevent nick changes or 
 * channel joins if the nick contains too many capital letters.
 *
 * Adds +U channel mode where U stands for "uppercase" and 
 * Config options for "nickcaps":
 *  minlen: how long a nick must be before maxcaps is applied.
 *	maxcaps: percentage of a nick that can be capital letters (integer 1 - 100).
 *  capsmap: A string of characters that will be considered capital letters.
 *
 *
 * TODO: because this requires inspircd it needs GPL version 2 license? I don't really know. I don't deal with licenses.
 */

#include "inspircd.h"

/* $ModDesc: Provides channel mode +U (prevent nick changes or channel joins if the nick contains too many capital letters) */

/* Defines the mode +U */
class ChannelNoAllCapsNicks : public SimpleChannelModeHandler
{
public:
	ChannelNoAllCapsNicks(Module* Creator) : SimpleChannelModeHandler(Creator, "nickcaps", 'U') { }
};


/* Module checks for capital letters in nicks and accepts/rejects
   them from joining channels or changing nicks.
*/
class NoAllCapsNicks : public Module 
{
	ChannelNoAllCapsNicks mode;
	unsigned int maxcaps;
	unsigned int minlen;
	char capsmap[256]; // note: capsmap is map where each character index is set to 0 or 1

public:
	NoAllCapsNicks() : mode(this)
	{
	}

	virtual ~NoAllCapsNicks()
	{
	}

	void init()
	{
		// get everything the module needs post-init
		OnRehash(NULL);
		
		// add channel mode
		ServerInstance->Modules->AddService(mode);

		// add events to capture from the server
		Implementation eventlist[] = { I_OnUserPreJoin, I_OnUserPreNick, I_OnRehash };
		ServerInstance->Modules->Attach(eventlist, this, sizeof(eventlist)/sizeof(Implementation));
	}
	
	/* Check if a user's nick is applicable on a channel with +U mode, denying entry if it isn't.
	*/
	virtual ModResult OnUserPreJoin(User* user, Channel* chan, const char* cname, std::string &privs, const std::string &keygiven)
	{
		if (!chan)
			/* Possible case (from module.h docs): channel has just been created and 
			   this is the first user joining... nothing to do.
			*/
			return MOD_RES_PASSTHRU;


		if (chan->IsModeSet(&mode) && this->DenyNick(user->nick)) {
			// 609 hijacked from m_joinflood.  TODO find a new magic error number
			user->WriteNumeric(609, "%s %s :Cannot join channel because nickname is invalid (+U). Nicknames longer than %d characters cannot contain %d%% capital letters or more.",user->nick.c_str(),chan->name.c_str(), this->minlen, this->maxcaps);
			return MOD_RES_DENY;
		}

		return MOD_RES_PASSTHRU;
	}

	/* Check when a user changes their nick, denying the change if a channel has the +U mode.
	*/
	virtual ModResult OnUserPreNick(User* user, const std::string &newnick)
	{
        // ignore OPERS
        if (!IS_OPER(user)) {

		    UserChanList chans(user->chans);

		    // check all the channels a user is a part of to see if any have +U
		    for (UCListIter i = chans.begin(); i != chans.end(); ++i)
		    {
			    Channel* c = *i;			

			    if (c->IsModeSet(&mode) && this->DenyNick(newnick)) {
				    // too many caps
				    user->WriteNumeric(ERR_CANTCHANGENICK, "%s :Can't change nickname as nickname is invalid while on channel %s (+U). Nicknames longer than %d characters cannot contain %d%% capital letters or more.", user->nick.c_str(), c->name.c_str(), this->minlen, this->maxcaps);
				    return MOD_RES_DENY;
			    }
		    }
		}
		return MOD_RES_PASSTHRU;
	}

	virtual void OnRehash(User* user)
	{
		ReadConf();
	}

	virtual Version GetVersion() 
	{
		// channel mode must be running on all linked servers (VF_COMMON)
		return Version("Provides channel mode +U to prevent nicks with too many capital letters.",  VF_COMMON);
	}
	
	/* Check a nick to see if it contains too many
	   capital letters as defined in the configuration file.

	   Returns: true if too many caps, false if not.
	*/
	bool DenyNick(const std::string &nick)
	{
		unsigned int total = 0;

		if (nick.length() <= this->minlen)
			// not enough characters to count
			return false;

		// tally the capital letters in the nick
		for (std::string::const_iterator i = nick.begin(); i != nick.end(); ++i)
		{
			// if capsmap index is set to 1, it will increment the total.
			total += this->capsmap[(unsigned char)*i];
		}

		// check if caps in nick is too much
		if (((total*100)/(int)nick.length()) >= this->maxcaps) {			
			return true;
		}

		return false;
	}	
	
	void ReadConf()
	{
		ConfigTag* tag = ServerInstance->Config->ConfValue("nickcaps");
		this->maxcaps = tag->getInt("maxcaps", 100); // percentage value as int
		this->minlen = tag->getInt("minlen", 4);

		// note (from above): this makes a map where each character is mapped to 0 or 1
		std::string hmap = tag->getString("capsmap", "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
		memset(capsmap, 0, sizeof(capsmap));
		for (std::string::iterator n = hmap.begin(); n != hmap.end(); n++)
			capsmap[(unsigned char)*n] = 1;

		if (this->maxcaps < 1 || this->maxcaps > 100)
		{
			ServerInstance->Logs->Log("CONFIG",DEFAULT, "<nickcaps:maxcaps> out of range, setting to default of 100.");
			maxcaps = 100;
		}
		if (this->minlen < 1 || this->minlen > MAXBUF-1) // TODO find out what MAXBUF is.
		{
			ServerInstance->Logs->Log("CONFIG",DEFAULT, "<nickcaps:minlen> out of range, setting to default of 4.");
			minlen = 4;
		}
		
	}
};

MODULE_INIT(NoAllCapsNicks)
