# SITE RESPONSE TOOL

### How to run:

**a) When used as independent tool:** 
~~~
SiteResponse.exe siteLayering.loc MotionX MotionZ MotionY OUT LOG

siteLayering.loc: Soil data for layered system 
MotionX: Acceleration time history in X direction 
MotionZ: Optional - Acceleration timehistory in Z direction 
MotionY: Optional - Acceleration timehistory in Y direction 
Program searches for MotionI.acc and MotionI.time

OUT & LOG: internal files

Folder must also include 
config.srtc: Configuration file to run SiteResponse.exe
~~~

**b) When used as part of SCEC BBP tool**
~~~
siteresponse.exe siteLayering.loc -bbp xxx.bbp OUT LOG

siteLayering.loc: Soil data for layered system
xxx.bbp: SCEC BBP motion file
OUT & LOG: internal files
~~~
