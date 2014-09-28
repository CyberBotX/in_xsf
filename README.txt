in_ncsf - NCSF Winamp Plugin
By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]

(If you need to email me, please include NCSF in the subject line.)

Special thanks to fincs for FeOS Sound System, as well as the DeSmuME
team for their open-source Nintendo DS emulator.

Version History
---------------
  v1.0 - 2013-03-25 - Initial Version
  v1.1 - 2013-04-02 - Fixed crash upon seeking backwards (also causes crash
                      in XMPlay on stopping).
                    - Added version number to the plugin description.
                    - Added 3 more interpolation options: B-spline, Hermite,
                      and Optimal.
  v1.2 - 2013-04-07 - Fixed crash if Winamp tries to access a file that no
                      longer exists, reported by Caitsith2.
                    - Utilized the SSEQ's volume from the INFO section.
                    - Minor optimizations.
  v1.3 - 2013-04-10 - Added more interpolation methods, cleanup of interpolation
                      code, and stopped using slope to determine points outside
                      the sample in an attempt to prevent clipping or popping.
                    - Made it so certain changes in the configuration dialog
                      will apply immediately after clicking OK.
                    - Minor optimizations.
  v1.4 - 2013-04-12 - Removed the Optimal, Lagrange, and Hermite interpolations,
                      added in 2nd-order Osculating interpolation.
                    - Changed handling of start/end of SWAV data to use the
                      closest data point instead of only the current one.
  v1.5 - 2013-04-18 - Implemented circular interpolation buffer to fix
                      interpolation, thanks to kode54 for the code.
  v1.6 - 2013-04-23 - Added Lanczos (Sinc) interpolation. Also modified Cosine
                      interpolation to use a lookup table instead.
  v1.7 - 2013-04-26 - Fixed Lanczos interpolation a bit (thanks to kode54), also
                      allowed the plugin to handle 32-bit samples.
v1.7.1 - 2013-04-26 - Minor update to scale the phase offset, thanks to kode54.
  v1.8 - 2013-05-07 - Replaced Lanczos window for Sinc with the Hann window.
                    - Added a ring buffer designed to work with SWAVs, replaces
                      kode54's implementation (still have to give him big thanks
                      for the original implementation).
                    - Corrected clamping issue which caused some clipping-like
                      effects.
v1.8.1 - 2014-06-17 - Reverted Sinc interpolation back to the Lanczos window.
  v1.9 - 2014-09-28 - Removed the Cosine, B-Spline, and Osculating
                      interpolations, added 4-point and 6-point Legrange
                      interpolations in their place.

This is a Winamp plugin to play NCSF files. NCSF is a PSF-style music format that
uses SDAT files from Nintendo DS ROMs as it's "program".

Contains:
*    in_ncsf.dll - The NCSF Winamp plugin
* zlib DLL v1.25 - Required by the plugin

To allow Winamp to use the plugin, place in_ncsf.dll into your Winamp plugins
directory and zlib1.dll into your Winamp directory (NOTE: NOT the plugins
directory). Once this has been done, start Winamp and you should be able to
play NCSF files.

If you wish to download from GitHub, you may do so from:

http://github.com/CyberBotX/in_xsf

This contains not only the above NCSF plugin, but also plugins for SNSF, GSF,
and 2SF. While the latter 3 are similar to existing plugins for those formats,
my versions utilize the same xSF framework that the NCSF plugin uses. In
addition, all plugins are much more modified than the versions they originate
from.

You may need the Microsoft Visual C++ 2010 Runtime, if you do not already have
it installed. You can obtain it here:

http://www.microsoft.com/en-us/download/details.aspx?id=5555

(NOTE: You will usually not require this runtime if you have Visual Studio installed,
or if you plan on installing Visual Studio. But you will need it if you do not have
Visual Studio installed.)

If you wish to compile the plugin yourself, you can do so with Microsoft Visual
Studio. You must use version 2010 or later. If you do not have Microsoft Visual
Studio, you can obtain the Express version of Microsoft Visual C++ from Microsoft, at:

http://www.microsoft.com/visualstudio/eng/downloads

You may use any Express version that is 2010 or later. Once you have Visual Studio
installed, you need to open the SDATStuff.sln file. If using a version of Visual
Studio later than 2010, you may be asked to upgrade the solution.

Of note, the utilities will not compile out of the box immediately. First, you need the
Winamp SDK. This is not included with the plugin's source code.  You can obtain the SDK
from here:

http://download.nullsoft.com/winamp/plugin-dev/WA5.55_SDK.exe

The zlib library is also required. zlib can be obtained from here:

http://www.zlib.net/

You need to download the "compiled DLL" version.

Once you have this, you need to modify the values of zlibRootDir and WinampSDKDir in
src\in_xsf_framework\common.props. Just replace ZLIBFIXME with the directory to which
you extracted zlib to, and replace WINAMPSDKFIXME with the directory to which you
installed the SDK to. Once this has been done, you can compile the solution.
