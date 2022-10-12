![SnemulDSDS](img/snemulDS-TWL.png)

NTR/TWL SDK: TGDS1.65

dsp: DSP games-only branch. Use TGDS1.65: branch for stable features.
Latest DSP release: https://bitbucket.org/Coto88/SnemulDS/get/dsp.zip

Changelog:

SnemulDS 0.6d DSP branch:
	- Plays DSP-only games! There are some sprite issues, and speed is a bit slower, but hey these play!

Note:
Make sure you tweak Options -> BG Priorities and use the best one available

-----------------------------------------------------------------------------------------------------------------------------------

Usage:
    - Download and copy all files from /release/arm7dldi-[ntr/twl], snemul.cfg as well, in SD:/ root. [NDS] mode is for DS mode, and [TWL] for DSi mode. 
	  If it prompts for overwrite: Yes to All. 
    - Create a /snes folder in SD: root, and put your homebrew games on it
	- SPC Playback: Optionally, create a /spc folder in SD: root, and put your SPC files on it. You can choose and play a SPC File in the "SPC Jukebox" option. 
	- [NTR]: Now open loader (internal, hbmenu or other), and run ToolchainGenericDS-multiboot.nds. Then select SnemulDS.nds from the menu, choose ARG (A button) and select the snes file to run.
	- [TWL]: Now open TWiLightMenu (you must set it up first, so you can run TWL mode apps), and run ToolchainGenericDS-multiboot.srl. Then select SnemulDS.srl from the menu, choose ARG (A button) and select the snes file to run.

-----------------------------------------------------------------------------------------------------------------------------------

Coto.
