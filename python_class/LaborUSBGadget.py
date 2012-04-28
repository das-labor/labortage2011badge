import usb.core
import usb.util

__all=['LaborUSBGadget']

class LaborUSBGadget(object):
  """
  die ist die oberklasse fuer alle Labor USB devices
  sie definiert ein paar default Werte
  unter anderem natuerlich die Vendorid und product ids
  ferner biete sie eine abstraction zu den usb-geraete, so, dass
  nur noch die funktionen zusammen gebaut werden muessen und man
  sich nicht mehr um die usb-details kuemmern muss
  """
  # 0x16c0
  LABOR_VENDOR_ID=5824
  # 0x05df
  LABOR_BADGE_ID=1503

  def __init__(self,ProductID):
    self.dev = usb.core.find(idVendor=self.LABOR_VENDOR_ID, idProduct=ProductID)
    
  def sendCTLMsg(self,request,data,value=0,index=0,timeout=100):
    """
    gibt einen int zurueck der angibt wieviele daten wirklich geschrieben wurden
    """
    return self.dev.ctrl_transfer(usb.TYPE_VENDOR|usb.RECIP_DEVICE|usb.ENDPOINT_OUT,request,value,index,data,timeout)

  def getCTLMsg(self,request,datalen,value=0,index=0,timeout=100):
    """
    gibt ein array mit den entsprechenden Werten zurueck
    """
    return self.dev.ctrl_transfer(usb.TYPE_VENDOR|usb.RECIP_DEVICE|usb.ENDPOINT_IN,request,value,index,datalen,timeout)
