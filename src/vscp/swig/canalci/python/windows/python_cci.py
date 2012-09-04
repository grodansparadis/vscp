# This file was created automatically by SWIG.
# Don't modify this file, modify the SWIG interface instead.
# This file is compatible with both classic and new-style classes.

import _python_cci

def _swig_setattr_nondynamic(self,class_type,name,value,static=1):
    if (name == "this"):
        if isinstance(value, class_type):
            self.__dict__[name] = value.this
            if hasattr(value,"thisown"): self.__dict__["thisown"] = value.thisown
            del value.thisown
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    if (not static) or hasattr(self,name) or (name == "thisown"):
        self.__dict__[name] = value
    else:
        raise AttributeError("You cannot add attributes to %s" % self)

def _swig_setattr(self,class_type,name,value):
    return _swig_setattr_nondynamic(self,class_type,name,value,0)

def _swig_getattr(self,class_type,name):
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError,name

import types
try:
    _object = types.ObjectType
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0
del types


USE_SHM_INTERFACE = _python_cci.USE_SHM_INTERFACE
USE_DLL_INTERFACE = _python_cci.USE_DLL_INTERFACE
MAX_PATH = _python_cci.MAX_PATH
class CCanalSuperWrapper(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CCanalSuperWrapper, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CCanalSuperWrapper, name)
    def __repr__(self):
        return "<%s.%s; proxy of C++ CCanalSuperWrapper instance at %s>" % (self.__class__.__module__, self.__class__.__name__, self.this,)
    def __init__(self, *args):
        _swig_setattr(self, CCanalSuperWrapper, 'this', _python_cci.new_CCanalSuperWrapper(*args))
        _swig_setattr(self, CCanalSuperWrapper, 'thisown', 1)
    def __del__(self, destroy=_python_cci.delete_CCanalSuperWrapper):
        try:
            if self.thisown: destroy(self)
        except: pass

    def doCmdOpen(*args): return _python_cci.CCanalSuperWrapper_doCmdOpen(*args)
    def doCmdClose(*args): return _python_cci.CCanalSuperWrapper_doCmdClose(*args)
    def doCmdNOOP(*args): return _python_cci.CCanalSuperWrapper_doCmdNOOP(*args)
    def doCmdGetLevel(*args): return _python_cci.CCanalSuperWrapper_doCmdGetLevel(*args)
    def doCmdSend(*args): return _python_cci.CCanalSuperWrapper_doCmdSend(*args)
    def doCmdReceive(*args): return _python_cci.CCanalSuperWrapper_doCmdReceive(*args)
    def doCmdDataAvailable(*args): return _python_cci.CCanalSuperWrapper_doCmdDataAvailable(*args)
    def doCmdStatus(*args): return _python_cci.CCanalSuperWrapper_doCmdStatus(*args)
    def doCmdStatistics(*args): return _python_cci.CCanalSuperWrapper_doCmdStatistics(*args)
    def doCmdFilter(*args): return _python_cci.CCanalSuperWrapper_doCmdFilter(*args)
    def doCmdMask(*args): return _python_cci.CCanalSuperWrapper_doCmdMask(*args)
    def doCmdBaudrate(*args): return _python_cci.CCanalSuperWrapper_doCmdBaudrate(*args)
    def doCmdVersion(*args): return _python_cci.CCanalSuperWrapper_doCmdVersion(*args)
    def doCmdDLLVersion(*args): return _python_cci.CCanalSuperWrapper_doCmdDLLVersion(*args)
    def doCmdVendorString(*args): return _python_cci.CCanalSuperWrapper_doCmdVendorString(*args)
    def new_canalMsg(*args): return _python_cci.CCanalSuperWrapper_new_canalMsg(*args)
    def setCanalMsgId(*args): return _python_cci.CCanalSuperWrapper_setCanalMsgId(*args)
    def getCanalMsgId(*args): return _python_cci.CCanalSuperWrapper_getCanalMsgId(*args)
    def setCanalMsgFlags(*args): return _python_cci.CCanalSuperWrapper_setCanalMsgFlags(*args)
    def getCanalMsgFlags(*args): return _python_cci.CCanalSuperWrapper_getCanalMsgFlags(*args)
    def setCanalMsgExtendedId(*args): return _python_cci.CCanalSuperWrapper_setCanalMsgExtendedId(*args)
    def isCanalMsgExtendedId(*args): return _python_cci.CCanalSuperWrapper_isCanalMsgExtendedId(*args)
    def setCanalMsgStandardId(*args): return _python_cci.CCanalSuperWrapper_setCanalMsgStandardId(*args)
    def isCanalMsgStandardId(*args): return _python_cci.CCanalSuperWrapper_isCanalMsgStandardId(*args)
    def setCanalMsgRemoteFrame(*args): return _python_cci.CCanalSuperWrapper_setCanalMsgRemoteFrame(*args)
    def isCanalMsgRemoteFrame(*args): return _python_cci.CCanalSuperWrapper_isCanalMsgRemoteFrame(*args)
    def setCanalMsgObId(*args): return _python_cci.CCanalSuperWrapper_setCanalMsgObId(*args)
    def getCanalObId(*args): return _python_cci.CCanalSuperWrapper_getCanalObId(*args)
    def setCanalMsgSizeData(*args): return _python_cci.CCanalSuperWrapper_setCanalMsgSizeData(*args)
    def getCanalMsgSizeData(*args): return _python_cci.CCanalSuperWrapper_getCanalMsgSizeData(*args)
    def setCanalMsgData(*args): return _python_cci.CCanalSuperWrapper_setCanalMsgData(*args)
    def getCanalMsgData(*args): return _python_cci.CCanalSuperWrapper_getCanalMsgData(*args)
    def new_canalStatistics(*args): return _python_cci.CCanalSuperWrapper_new_canalStatistics(*args)
    def getCanalStatisticsReceiveFrames(*args): return _python_cci.CCanalSuperWrapper_getCanalStatisticsReceiveFrames(*args)
    def getCanalStatisticsTransmitFrames(*args): return _python_cci.CCanalSuperWrapper_getCanalStatisticsTransmitFrames(*args)
    def getCanalStatisticsReceiveData(*args): return _python_cci.CCanalSuperWrapper_getCanalStatisticsReceiveData(*args)
    def getCanalStatisticsTransmitData(*args): return _python_cci.CCanalSuperWrapper_getCanalStatisticsTransmitData(*args)
    def getCanalStatisticsOverruns(*args): return _python_cci.CCanalSuperWrapper_getCanalStatisticsOverruns(*args)
    def getCanalStatisticsBusWarnings(*args): return _python_cci.CCanalSuperWrapper_getCanalStatisticsBusWarnings(*args)
    def getCanalStatisticsBusOff(*args): return _python_cci.CCanalSuperWrapper_getCanalStatisticsBusOff(*args)
    def new_canalStatus(*args): return _python_cci.CCanalSuperWrapper_new_canalStatus(*args)
    def getcanalStatusChannelStatus(*args): return _python_cci.CCanalSuperWrapper_getcanalStatusChannelStatus(*args)
    def new_devItem(*args): return _python_cci.CCanalSuperWrapper_new_devItem(*args)
    def setdevItemId(*args): return _python_cci.CCanalSuperWrapper_setdevItemId(*args)
    def getdevItemId(*args): return _python_cci.CCanalSuperWrapper_getdevItemId(*args)
    def setdevItemRegId(*args): return _python_cci.CCanalSuperWrapper_setdevItemRegId(*args)
    def getdevItemRegId(*args): return _python_cci.CCanalSuperWrapper_getdevItemRegId(*args)
    def setdevItemName(*args): return _python_cci.CCanalSuperWrapper_setdevItemName(*args)
    def getdevItemName(*args): return _python_cci.CCanalSuperWrapper_getdevItemName(*args)
    def setdevItemPath(*args): return _python_cci.CCanalSuperWrapper_setdevItemPath(*args)
    def getdevItemPath(*args): return _python_cci.CCanalSuperWrapper_getdevItemPath(*args)
    def setdevItemDeviceString(*args): return _python_cci.CCanalSuperWrapper_setdevItemDeviceString(*args)
    def getdevItemDeviceString(*args): return _python_cci.CCanalSuperWrapper_getdevItemDeviceString(*args)
    def setdevItemFlags(*args): return _python_cci.CCanalSuperWrapper_setdevItemFlags(*args)
    def getdevItemFlags(*args): return _python_cci.CCanalSuperWrapper_getdevItemFlags(*args)
    def setdevItemInBufSize(*args): return _python_cci.CCanalSuperWrapper_setdevItemInBufSize(*args)
    def getdevItemInBufSize(*args): return _python_cci.CCanalSuperWrapper_getdevItemInBufSize(*args)
    def setdevItemOutBufSize(*args): return _python_cci.CCanalSuperWrapper_setdevItemOutBufSize(*args)
    def getdevItemOutBufSize(*args): return _python_cci.CCanalSuperWrapper_getdevItemOutBufSize(*args)
    def setdevItemFilter(*args): return _python_cci.CCanalSuperWrapper_setdevItemFilter(*args)
    def getdevItemFilter(*args): return _python_cci.CCanalSuperWrapper_getdevItemFilter(*args)
    def setdevItemMask(*args): return _python_cci.CCanalSuperWrapper_setdevItemMask(*args)
    def getdevItemMask(*args): return _python_cci.CCanalSuperWrapper_getdevItemMask(*args)

class CCanalSuperWrapperPtr(CCanalSuperWrapper):
    def __init__(self, this):
        _swig_setattr(self, CCanalSuperWrapper, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CCanalSuperWrapper, 'thisown', 0)
        _swig_setattr(self, CCanalSuperWrapper,self.__class__,CCanalSuperWrapper)
_python_cci.CCanalSuperWrapper_swigregister(CCanalSuperWrapperPtr)

class canalMsg(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, canalMsg, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, canalMsg, name)
    def __repr__(self):
        return "<%s.%s; proxy of C++ canalMsg instance at %s>" % (self.__class__.__module__, self.__class__.__name__, self.this,)
    __swig_setmethods__["flags"] = _python_cci.canalMsg_flags_set
    __swig_getmethods__["flags"] = _python_cci.canalMsg_flags_get
    if _newclass:flags = property(_python_cci.canalMsg_flags_get, _python_cci.canalMsg_flags_set)
    __swig_setmethods__["obid"] = _python_cci.canalMsg_obid_set
    __swig_getmethods__["obid"] = _python_cci.canalMsg_obid_get
    if _newclass:obid = property(_python_cci.canalMsg_obid_get, _python_cci.canalMsg_obid_set)
    __swig_setmethods__["id"] = _python_cci.canalMsg_id_set
    __swig_getmethods__["id"] = _python_cci.canalMsg_id_get
    if _newclass:id = property(_python_cci.canalMsg_id_get, _python_cci.canalMsg_id_set)
    __swig_setmethods__["sizeData"] = _python_cci.canalMsg_sizeData_set
    __swig_getmethods__["sizeData"] = _python_cci.canalMsg_sizeData_get
    if _newclass:sizeData = property(_python_cci.canalMsg_sizeData_get, _python_cci.canalMsg_sizeData_set)
    __swig_setmethods__["data"] = _python_cci.canalMsg_data_set
    __swig_getmethods__["data"] = _python_cci.canalMsg_data_get
    if _newclass:data = property(_python_cci.canalMsg_data_get, _python_cci.canalMsg_data_set)
    __swig_setmethods__["timestamp"] = _python_cci.canalMsg_timestamp_set
    __swig_getmethods__["timestamp"] = _python_cci.canalMsg_timestamp_get
    if _newclass:timestamp = property(_python_cci.canalMsg_timestamp_get, _python_cci.canalMsg_timestamp_set)
    def __init__(self, *args):
        _swig_setattr(self, canalMsg, 'this', _python_cci.new_canalMsg(*args))
        _swig_setattr(self, canalMsg, 'thisown', 1)
    def __del__(self, destroy=_python_cci.delete_canalMsg):
        try:
            if self.thisown: destroy(self)
        except: pass


class canalMsgPtr(canalMsg):
    def __init__(self, this):
        _swig_setattr(self, canalMsg, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, canalMsg, 'thisown', 0)
        _swig_setattr(self, canalMsg,self.__class__,canalMsg)
_python_cci.canalMsg_swigregister(canalMsgPtr)

class canalStatistics(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, canalStatistics, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, canalStatistics, name)
    def __repr__(self):
        return "<%s.%s; proxy of C++ canalStatistics instance at %s>" % (self.__class__.__module__, self.__class__.__name__, self.this,)
    __swig_setmethods__["cntReceiveFrames"] = _python_cci.canalStatistics_cntReceiveFrames_set
    __swig_getmethods__["cntReceiveFrames"] = _python_cci.canalStatistics_cntReceiveFrames_get
    if _newclass:cntReceiveFrames = property(_python_cci.canalStatistics_cntReceiveFrames_get, _python_cci.canalStatistics_cntReceiveFrames_set)
    __swig_setmethods__["cntTransmitFrames"] = _python_cci.canalStatistics_cntTransmitFrames_set
    __swig_getmethods__["cntTransmitFrames"] = _python_cci.canalStatistics_cntTransmitFrames_get
    if _newclass:cntTransmitFrames = property(_python_cci.canalStatistics_cntTransmitFrames_get, _python_cci.canalStatistics_cntTransmitFrames_set)
    __swig_setmethods__["cntReceiveData"] = _python_cci.canalStatistics_cntReceiveData_set
    __swig_getmethods__["cntReceiveData"] = _python_cci.canalStatistics_cntReceiveData_get
    if _newclass:cntReceiveData = property(_python_cci.canalStatistics_cntReceiveData_get, _python_cci.canalStatistics_cntReceiveData_set)
    __swig_setmethods__["cntTransmitData"] = _python_cci.canalStatistics_cntTransmitData_set
    __swig_getmethods__["cntTransmitData"] = _python_cci.canalStatistics_cntTransmitData_get
    if _newclass:cntTransmitData = property(_python_cci.canalStatistics_cntTransmitData_get, _python_cci.canalStatistics_cntTransmitData_set)
    __swig_setmethods__["cntOverruns"] = _python_cci.canalStatistics_cntOverruns_set
    __swig_getmethods__["cntOverruns"] = _python_cci.canalStatistics_cntOverruns_get
    if _newclass:cntOverruns = property(_python_cci.canalStatistics_cntOverruns_get, _python_cci.canalStatistics_cntOverruns_set)
    __swig_setmethods__["cntBusWarnings"] = _python_cci.canalStatistics_cntBusWarnings_set
    __swig_getmethods__["cntBusWarnings"] = _python_cci.canalStatistics_cntBusWarnings_get
    if _newclass:cntBusWarnings = property(_python_cci.canalStatistics_cntBusWarnings_get, _python_cci.canalStatistics_cntBusWarnings_set)
    __swig_setmethods__["cntBusOff"] = _python_cci.canalStatistics_cntBusOff_set
    __swig_getmethods__["cntBusOff"] = _python_cci.canalStatistics_cntBusOff_get
    if _newclass:cntBusOff = property(_python_cci.canalStatistics_cntBusOff_get, _python_cci.canalStatistics_cntBusOff_set)
    def __init__(self, *args):
        _swig_setattr(self, canalStatistics, 'this', _python_cci.new_canalStatistics(*args))
        _swig_setattr(self, canalStatistics, 'thisown', 1)
    def __del__(self, destroy=_python_cci.delete_canalStatistics):
        try:
            if self.thisown: destroy(self)
        except: pass


class canalStatisticsPtr(canalStatistics):
    def __init__(self, this):
        _swig_setattr(self, canalStatistics, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, canalStatistics, 'thisown', 0)
        _swig_setattr(self, canalStatistics,self.__class__,canalStatistics)
_python_cci.canalStatistics_swigregister(canalStatisticsPtr)

class canalStatus(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, canalStatus, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, canalStatus, name)
    def __repr__(self):
        return "<%s.%s; proxy of C++ canalStatus instance at %s>" % (self.__class__.__module__, self.__class__.__name__, self.this,)
    __swig_setmethods__["channel_status"] = _python_cci.canalStatus_channel_status_set
    __swig_getmethods__["channel_status"] = _python_cci.canalStatus_channel_status_get
    if _newclass:channel_status = property(_python_cci.canalStatus_channel_status_get, _python_cci.canalStatus_channel_status_set)
    def __init__(self, *args):
        _swig_setattr(self, canalStatus, 'this', _python_cci.new_canalStatus(*args))
        _swig_setattr(self, canalStatus, 'thisown', 1)
    def __del__(self, destroy=_python_cci.delete_canalStatus):
        try:
            if self.thisown: destroy(self)
        except: pass


class canalStatusPtr(canalStatus):
    def __init__(self, this):
        _swig_setattr(self, canalStatus, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, canalStatus, 'thisown', 0)
        _swig_setattr(self, canalStatus,self.__class__,canalStatus)
_python_cci.canalStatus_swigregister(canalStatusPtr)

class devItem(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, devItem, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, devItem, name)
    def __repr__(self):
        return "<%s.%s; proxy of C++ devItem instance at %s>" % (self.__class__.__module__, self.__class__.__name__, self.this,)
    __swig_setmethods__["id"] = _python_cci.devItem_id_set
    __swig_getmethods__["id"] = _python_cci.devItem_id_get
    if _newclass:id = property(_python_cci.devItem_id_get, _python_cci.devItem_id_set)
    __swig_setmethods__["regid"] = _python_cci.devItem_regid_set
    __swig_getmethods__["regid"] = _python_cci.devItem_regid_get
    if _newclass:regid = property(_python_cci.devItem_regid_get, _python_cci.devItem_regid_set)
    __swig_setmethods__["name"] = _python_cci.devItem_name_set
    __swig_getmethods__["name"] = _python_cci.devItem_name_get
    if _newclass:name = property(_python_cci.devItem_name_get, _python_cci.devItem_name_set)
    __swig_setmethods__["path"] = _python_cci.devItem_path_set
    __swig_getmethods__["path"] = _python_cci.devItem_path_get
    if _newclass:path = property(_python_cci.devItem_path_get, _python_cci.devItem_path_set)
    __swig_setmethods__["deviceStr"] = _python_cci.devItem_deviceStr_set
    __swig_getmethods__["deviceStr"] = _python_cci.devItem_deviceStr_get
    if _newclass:deviceStr = property(_python_cci.devItem_deviceStr_get, _python_cci.devItem_deviceStr_set)
    __swig_setmethods__["flags"] = _python_cci.devItem_flags_set
    __swig_getmethods__["flags"] = _python_cci.devItem_flags_get
    if _newclass:flags = property(_python_cci.devItem_flags_get, _python_cci.devItem_flags_set)
    __swig_setmethods__["inbufsize"] = _python_cci.devItem_inbufsize_set
    __swig_getmethods__["inbufsize"] = _python_cci.devItem_inbufsize_get
    if _newclass:inbufsize = property(_python_cci.devItem_inbufsize_get, _python_cci.devItem_inbufsize_set)
    __swig_setmethods__["outbufsize"] = _python_cci.devItem_outbufsize_set
    __swig_getmethods__["outbufsize"] = _python_cci.devItem_outbufsize_get
    if _newclass:outbufsize = property(_python_cci.devItem_outbufsize_get, _python_cci.devItem_outbufsize_set)
    __swig_setmethods__["filter"] = _python_cci.devItem_filter_set
    __swig_getmethods__["filter"] = _python_cci.devItem_filter_get
    if _newclass:filter = property(_python_cci.devItem_filter_get, _python_cci.devItem_filter_set)
    __swig_setmethods__["mask"] = _python_cci.devItem_mask_set
    __swig_getmethods__["mask"] = _python_cci.devItem_mask_get
    if _newclass:mask = property(_python_cci.devItem_mask_get, _python_cci.devItem_mask_set)
    def __init__(self, *args):
        _swig_setattr(self, devItem, 'this', _python_cci.new_devItem(*args))
        _swig_setattr(self, devItem, 'thisown', 1)
    def __del__(self, destroy=_python_cci.delete_devItem):
        try:
            if self.thisown: destroy(self)
        except: pass


class devItemPtr(devItem):
    def __init__(self, this):
        _swig_setattr(self, devItem, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, devItem, 'thisown', 0)
        _swig_setattr(self, devItem,self.__class__,devItem)
_python_cci.devItem_swigregister(devItemPtr)


