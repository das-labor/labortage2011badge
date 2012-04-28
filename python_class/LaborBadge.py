#!/usr/bin/env python


from  LaborUSBGadget import LaborUSBGadget 
from time import sleep

class LaborBadge(LaborUSBGadget):
  """
  Dies ist die klasse fuer das LaborTage 2011 Badge
  ein kleines blinkendevice
  """
  commands={'CUSTOM_RQ_SET_RED':      0x03,
            'CUSTOM_RQ_SET_GREEN':    0x04,
            'CUSTOM_RQ_SET_BLUE':     0x05,
            'CUSTOM_RQ_SET_RGB':      0x06,
            'CUSTOM_RQ_GET_RGB':      0x07,
            'CUSTOM_RQ_FADE_RGB':     0x08,
            'CUSTOM_RQ_READ_MEM':     0x10,
            'CUSTOM_RQ_WRITE_MEM':    0x11,
            'CUSTOM_RQ_READ_FLASH':   0x12,
            'CUSTOM_RQ_EXEC_SPM':     0x13,
            'CUSTOM_RQ_RESET':        0x14,
            'CUSTOM_RQ_READ_BUTTON':  0x15,
            'CUSTOM_RQ_READ_TMPSENS': 0x16}


  def __init__(self):
    self.starttempUSB=230
    self.starttempHost=-40
    LaborUSBGadget.__init__(self,LaborUSBGadget.LABOR_BADGE_ID)

  def getTemprature(self):
    """
    Das badge kann die raumtemperatur auswerten - lets have a look
    also im datenblatt vom attiny45 ist folgende tabelle abgebildet
    -40   +25    +85
    230   300    370
    und die Aussagen:
     * The measured voltage has a linear relationship to the temperature
     * The sensitivity is approximately 1 LSB / Grad C

    # linear
    a*(-40)+b = 230
    a*(25) + b = 300
    a*(85) + b = 370

    -40a+b-230=25a+b-300
    65a=70

    a=1.07692307692307692307

    -40a-230=85a-370
    a=1.12000000000000000000

    25a-300=85a-370
    a=1.16666666666666666666

    summe ueber alle a und dann druch drei ;)
    1.12119658119658119657 - das ist jetzt unser linearer korrekturfaktor k

    Bitte bitte selber justieren! das ding ist wirklich ungenau
    """
    tmptemp=self.getCTLMsg(self.commands['CUSTOM_RQ_READ_TMPSENS'],2)
    value=(tmptemp[1]<<8)+tmptemp[0]
    #a=1
    k=1.12119658119658119657
    return float(value-float(self.starttempUSB)+self.starttempHost)/k
    
  def setColor(self,r=0,g=0,b=0):
    """
    eine methode zum setzen aller farbwerte gleichzeitig
    die range ist dabei 0-65565 beide inklusive
    alternativ kann auch ein dict uebergeben werden
    hierbei muessen die schluessel
    red,green,blue vorhanden sein
    """
    # check for range
    if isinstance(r,type({})):
      tmpr=0
      tmpg=0
      tmpb=0
      if r.has_key('green'):
        tmpg=r['green']
      if r.has_key('blue'):
        tmpb=r['blue']
      if r.has_key('red'):
        tmpr=r['red']
      r=tmpr
      g=tmpg
      b=tmpb

    if r in xrange(2**16) and g in xrange(2**16) and b in xrange(2**16):
      # und an der stelle brauchen wir die byterepresentation bigendien stream
      tmparray=[]
      tmparray.append(r & 0xff)
      tmparray.append(r >> 8)
      tmparray.append(g & 0xff)
      tmparray.append(g >> 8)
      tmparray.append(b & 0xff)
      tmparray.append(b >> 8)
      ret=self.sendCTLMsg(self.commands['CUSTOM_RQ_SET_RGB'],tmparray)


  def fadeToColor(self,fadetocolor,speed):
    """
    """
    currentc=self.getColor()
    diffcolor={}
    tmparray=[]
    for v in currentc:
      diffcolor[v]=fadetocolor[v]-currentc[v]
      if diffcolor[v] < 0 :
        print diffcolor
        diffcolor[v]=diffcolor[v]+0xffff
        print diffcolor
      diffcolor[v]=diffcolor[v]/speed
      if diffcolor[v] ==0:
        diffcolor[v]=1

    tmparray.append(diffcolor['red'] & 0xff)
    tmparray.append(diffcolor['red'] >> 8)
    tmparray.append(diffcolor['green'] & 0xff)
    tmparray.append(diffcolor['green'] >> 8)
    tmparray.append(diffcolor['blue'] & 0xff)
    tmparray.append(diffcolor['blue'] >> 8)

    print diffcolor
    print self.sendCTLMsg(self.commands['CUSTOM_RQ_FADE_RGB'],tmparray,speed)
    

  def blink(self,blinkcolor,speed,count):
    """
    macht einen farbwechsel in der angegebene geschwindigkeit.
    dabei ist die startfarbe die aktuelle farbe und die endfabe die
    uebergebene
    """
    pass

  def getColor(self):
    """
    die aktuell eingestellt farbe ausgeben
    """
    result={}
    ret=self.getCTLMsg(self.commands['CUSTOM_RQ_GET_RGB'],6)
    result["red"]=(ret[1]<<8)+ret[0]
    result["green"]=(ret[3]<<8)+ret[2]
    result["blue"]=(ret[5]<<8)+ret[4]
    return result


if __name__ == "__main__":
  blinkcolor={}
  blinkcolor2={}
  blinkcolor['green']=0xffff
  blinkcolor['red']=0x0000
  blinkcolor['blue']=0xffff
  blinkcolor2['green']=0x0000
  blinkcolor2['red']=0x0000
  blinkcolor2['blue']=0xffff
  f=LaborBadge()
  f.setColor(blinkcolor2)
  sleep(2)
  f.fadeToColor(blinkcolor,100)
  sleep(2)
  f.fadeToColor(blinkcolor2,100)

  print f.getTemprature()
  print f.getColor()
