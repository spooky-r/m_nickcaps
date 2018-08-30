# InspIRCd 2.0 branch module "m_nickcaps" #
For two cool dudes named ben.
## Description ##
"m_nickcaps" blocks changing nicks in or joining a channel with +U if the nick has too many capital letters.  
(This module also allows opers and channel ops to freely change their nicks whether the mode is set or not.)
## HowTo Guide ##
To install:
- Clone or download the repository.
- Symlink or copy the "m_nickcaps.cpp" file to the inspircd 2.0 branch /src/modules directory.
- Make and install (no need to run ./configure if already configured).

Configuring:
- Add module to module.conf `<module name="m_nickcaps.so">`
- Edit module configuration for `<nickcaps>`:
  - "minlen": minimum length a nick can be before being checked for capital letters. A "minlen" of 4 will check a nick like "test" but not "tim"
  - "maxcaps": percentage of a nick, beyond minlen, that cannot be capital letters. A "maxcaps" of 50% will not allow a nick like "ABcd" and 51% would allow it.
  - "capsmap": a string of characters considered capital letters.
- Set any channel +U as needed. Invalid nicks already in the channel will remain, however.

