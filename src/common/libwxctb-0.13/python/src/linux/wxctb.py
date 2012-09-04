# This file was created automatically by SWIG 1.3.28.
# Don't modify this file, modify the SWIG interface instead.

import _wxctb
import new
new_instancemethod = new.instancemethod
def _swig_setattr_nondynamic(self,class_type,name,value,static=1):
    if (name == "thisown"): return self.this.own(value)
    if (name == "this"):
        if type(value).__name__ == 'PySwigObject':
            self.__dict__[name] = value
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    if (not static) or hasattr(self,name):
        self.__dict__[name] = value
    else:
        raise AttributeError("You cannot add attributes to %s" % self)

def _swig_setattr(self,class_type,name,value):
    return _swig_setattr_nondynamic(self,class_type,name,value,0)

def _swig_getattr(self,class_type,name):
    if (name == "thisown"): return self.this.own()
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


def _swig_setattr_nondynamic_method(set):
    def set_attr(self,name,value):
        if (name == "thisown"): return self.this.own(value)
        if hasattr(self,name) or (name == "this"):
            set(self,name,value)
        else:
            raise AttributeError("You cannot add attributes to %s" % self)
    return set_attr



new_intp = _wxctb.new_intp

copy_intp = _wxctb.copy_intp

delete_intp = _wxctb.delete_intp

intp_assign = _wxctb.intp_assign

intp_value = _wxctb.intp_value
class timer_control(object):
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self): raise AttributeError, "No constructor defined"
    def __repr__(self):
        try: strthis = "at 0x%x" %( self.this, ) 
        except: strthis = "" 
        return "<%s.%s; proxy of C++ timer_control instance %s>" % (self.__class__.__module__, self.__class__.__name__, strthis,)
    usecs = property(_wxctb.timer_control_usecs_get, _wxctb.timer_control_usecs_set)
    exitflag = property(_wxctb.timer_control_exitflag_get, _wxctb.timer_control_exitflag_set)
    exitfnc = property(_wxctb.timer_control_exitfnc_get, _wxctb.timer_control_exitfnc_set)
_wxctb.timer_control_swigregister(timer_control)

class timer(object):
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __repr__(self):
        try: strthis = "at 0x%x" %( self.this, ) 
        except: strthis = "" 
        return "<%s.%s; proxy of C++ timer instance %s>" % (self.__class__.__module__, self.__class__.__name__, strthis,)
    def __init__(self, *args, **kwargs):
        this = _wxctb.new_timer(*args, **kwargs)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _wxctb.delete_timer
    __del__ = lambda self : None;
    def start(*args, **kwargs): return _wxctb.timer_start(*args, **kwargs)
    def stop(*args, **kwargs): return _wxctb.timer_stop(*args, **kwargs)
_wxctb.timer_swigregister(timer)


sleepms = _wxctb.sleepms
CTB_RESET = _wxctb.CTB_RESET
class wxIOBase(object):
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self): raise AttributeError, "No constructor defined"
    def __repr__(self):
        try: strthis = "at 0x%x" %( self.this, ) 
        except: strthis = "" 
        return "<%s.%s; proxy of C++ wxIOBase instance %s>" % (self.__class__.__module__, self.__class__.__name__, strthis,)
    __swig_destroy__ = _wxctb.delete_wxIOBase
    __del__ = lambda self : None;
    def ClassName(*args, **kwargs): return _wxctb.wxIOBase_ClassName(*args, **kwargs)
    def Close(*args, **kwargs): return _wxctb.wxIOBase_Close(*args, **kwargs)
    def Ioctl(*args, **kwargs): return _wxctb.wxIOBase_Ioctl(*args, **kwargs)
    def IsOpen(*args, **kwargs): return _wxctb.wxIOBase_IsOpen(*args, **kwargs)
    def Open(*args, **kwargs): return _wxctb.wxIOBase_Open(*args, **kwargs)
    def PutBack(*args, **kwargs): return _wxctb.wxIOBase_PutBack(*args, **kwargs)
    def Read(*args, **kwargs): return _wxctb.wxIOBase_Read(*args, **kwargs)
    def ReadUntilEOS(*args, **kwargs): return _wxctb.wxIOBase_ReadUntilEOS(*args, **kwargs)
    def Readv(*args, **kwargs): return _wxctb.wxIOBase_Readv(*args, **kwargs)
    def Write(*args, **kwargs): return _wxctb.wxIOBase_Write(*args, **kwargs)
    def Writev(*args, **kwargs): return _wxctb.wxIOBase_Writev(*args, **kwargs)
_wxctb.wxIOBase_swigregister(wxIOBase)

wxBAUD_150 = _wxctb.wxBAUD_150
wxBAUD_300 = _wxctb.wxBAUD_300
wxBAUD_600 = _wxctb.wxBAUD_600
wxBAUD_1200 = _wxctb.wxBAUD_1200
wxBAUD_2400 = _wxctb.wxBAUD_2400
wxBAUD_4800 = _wxctb.wxBAUD_4800
wxBAUD_9600 = _wxctb.wxBAUD_9600
wxBAUD_19200 = _wxctb.wxBAUD_19200
wxBAUD_38400 = _wxctb.wxBAUD_38400
wxBAUD_57600 = _wxctb.wxBAUD_57600
wxBAUD_115200 = _wxctb.wxBAUD_115200
wxBAUD_230400 = _wxctb.wxBAUD_230400
wxBAUD_460800 = _wxctb.wxBAUD_460800
wxBAUD_921600 = _wxctb.wxBAUD_921600
wxPARITY_NONE = _wxctb.wxPARITY_NONE
wxPARITY_ODD = _wxctb.wxPARITY_ODD
wxPARITY_EVEN = _wxctb.wxPARITY_EVEN
wxPARITY_MARK = _wxctb.wxPARITY_MARK
wxPARITY_SPACE = _wxctb.wxPARITY_SPACE
wxSERIAL_LINESTATE_DCD = _wxctb.wxSERIAL_LINESTATE_DCD
wxSERIAL_LINESTATE_CTS = _wxctb.wxSERIAL_LINESTATE_CTS
wxSERIAL_LINESTATE_DSR = _wxctb.wxSERIAL_LINESTATE_DSR
wxSERIAL_LINESTATE_DTR = _wxctb.wxSERIAL_LINESTATE_DTR
wxSERIAL_LINESTATE_RING = _wxctb.wxSERIAL_LINESTATE_RING
wxSERIAL_LINESTATE_RTS = _wxctb.wxSERIAL_LINESTATE_RTS
wxSERIAL_LINESTATE_NULL = _wxctb.wxSERIAL_LINESTATE_NULL
class wxSerialPort_DCS(object):
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __repr__(self):
        try: strthis = "at 0x%x" %( self.this, ) 
        except: strthis = "" 
        return "<%s.%s; proxy of C++ wxSerialPort_DCS instance %s>" % (self.__class__.__module__, self.__class__.__name__, strthis,)
    baud = property(_wxctb.wxSerialPort_DCS_baud_get, _wxctb.wxSerialPort_DCS_baud_set)
    parity = property(_wxctb.wxSerialPort_DCS_parity_get, _wxctb.wxSerialPort_DCS_parity_set)
    wordlen = property(_wxctb.wxSerialPort_DCS_wordlen_get, _wxctb.wxSerialPort_DCS_wordlen_set)
    stopbits = property(_wxctb.wxSerialPort_DCS_stopbits_get, _wxctb.wxSerialPort_DCS_stopbits_set)
    rtscts = property(_wxctb.wxSerialPort_DCS_rtscts_get, _wxctb.wxSerialPort_DCS_rtscts_set)
    xonxoff = property(_wxctb.wxSerialPort_DCS_xonxoff_get, _wxctb.wxSerialPort_DCS_xonxoff_set)
    buf = property(_wxctb.wxSerialPort_DCS_buf_get, _wxctb.wxSerialPort_DCS_buf_set)
    def __init__(self, *args, **kwargs):
        this = _wxctb.new_wxSerialPort_DCS(*args, **kwargs)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _wxctb.delete_wxSerialPort_DCS
    __del__ = lambda self : None;
    def GetSettings(*args, **kwargs): return _wxctb.wxSerialPort_DCS_GetSettings(*args, **kwargs)
_wxctb.wxSerialPort_DCS_swigregister(wxSerialPort_DCS)

class wxSerialPort_EINFO(object):
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __repr__(self):
        try: strthis = "at 0x%x" %( self.this, ) 
        except: strthis = "" 
        return "<%s.%s; proxy of C++ wxSerialPort_EINFO instance %s>" % (self.__class__.__module__, self.__class__.__name__, strthis,)
    brk = property(_wxctb.wxSerialPort_EINFO_brk_get, _wxctb.wxSerialPort_EINFO_brk_set)
    frame = property(_wxctb.wxSerialPort_EINFO_frame_get, _wxctb.wxSerialPort_EINFO_frame_set)
    overrun = property(_wxctb.wxSerialPort_EINFO_overrun_get, _wxctb.wxSerialPort_EINFO_overrun_set)
    parity = property(_wxctb.wxSerialPort_EINFO_parity_get, _wxctb.wxSerialPort_EINFO_parity_set)
    def __init__(self, *args, **kwargs):
        this = _wxctb.new_wxSerialPort_EINFO(*args, **kwargs)
        try: self.this.append(this)
        except: self.this = this
_wxctb.wxSerialPort_EINFO_swigregister(wxSerialPort_EINFO)

CTB_SER_GETEINFO = _wxctb.CTB_SER_GETEINFO
CTB_SER_GETBRK = _wxctb.CTB_SER_GETBRK
CTB_SER_GETFRM = _wxctb.CTB_SER_GETFRM
CTB_SER_GETOVR = _wxctb.CTB_SER_GETOVR
CTB_SER_GETPAR = _wxctb.CTB_SER_GETPAR
CTB_SER_GETINQUE = _wxctb.CTB_SER_GETINQUE
class wxSerialPort_x(wxIOBase):
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self): raise AttributeError, "No constructor defined"
    def __repr__(self):
        try: strthis = "at 0x%x" %( self.this, ) 
        except: strthis = "" 
        return "<%s.%s; proxy of C++ wxSerialPort_x instance %s>" % (self.__class__.__module__, self.__class__.__name__, strthis,)
    __swig_destroy__ = _wxctb.delete_wxSerialPort_x
    __del__ = lambda self : None;
    def ClassName(*args, **kwargs): return _wxctb.wxSerialPort_x_ClassName(*args, **kwargs)
    def ChangeLineState(*args, **kwargs): return _wxctb.wxSerialPort_x_ChangeLineState(*args, **kwargs)
    def ClrLineState(*args, **kwargs): return _wxctb.wxSerialPort_x_ClrLineState(*args, **kwargs)
    def GetLineState(*args, **kwargs): return _wxctb.wxSerialPort_x_GetLineState(*args, **kwargs)
    def GetSettingsAsString(*args, **kwargs): return _wxctb.wxSerialPort_x_GetSettingsAsString(*args, **kwargs)
    def Ioctl(*args, **kwargs): return _wxctb.wxSerialPort_x_Ioctl(*args, **kwargs)
    def SendBreak(*args, **kwargs): return _wxctb.wxSerialPort_x_SendBreak(*args, **kwargs)
    def SetBaudRate(*args, **kwargs): return _wxctb.wxSerialPort_x_SetBaudRate(*args, **kwargs)
    def SetLineState(*args, **kwargs): return _wxctb.wxSerialPort_x_SetLineState(*args, **kwargs)
_wxctb.wxSerialPort_x_swigregister(wxSerialPort_x)

wxCOM1 = "/dev/ttyS0"
wxCOM2 = "/dev/ttyS1"
wxCOM3 = "/dev/ttyS2"
wxCOM4 = "/dev/ttyS3"
wxCOM5 = "/dev/ttyS4"
wxCOM6 = "/dev/ttyS5"
wxCOM7 = "/dev/ttyS6"
wxCOM8 = "/dev/ttyS7"
wxCOM9 = "/dev/ttyS8"

class wxSerialPort(wxSerialPort_x):
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __repr__(self):
        try: strthis = "at 0x%x" %( self.this, ) 
        except: strthis = "" 
        return "<%s.%s; proxy of C++ wxSerialPort instance %s>" % (self.__class__.__module__, self.__class__.__name__, strthis,)
    def __init__(self, *args, **kwargs):
        this = _wxctb.new_wxSerialPort(*args, **kwargs)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _wxctb.delete_wxSerialPort
    __del__ = lambda self : None;
    def ChangeLineState(*args, **kwargs): return _wxctb.wxSerialPort_ChangeLineState(*args, **kwargs)
    def ClrLineState(*args, **kwargs): return _wxctb.wxSerialPort_ClrLineState(*args, **kwargs)
    def GetLineState(*args, **kwargs): return _wxctb.wxSerialPort_GetLineState(*args, **kwargs)
    def Ioctl(*args, **kwargs): return _wxctb.wxSerialPort_Ioctl(*args, **kwargs)
    def IsOpen(*args, **kwargs): return _wxctb.wxSerialPort_IsOpen(*args, **kwargs)
    def Read(*args, **kwargs): return _wxctb.wxSerialPort_Read(*args, **kwargs)
    def SendBreak(*args, **kwargs): return _wxctb.wxSerialPort_SendBreak(*args, **kwargs)
    def SetBaudRate(*args, **kwargs): return _wxctb.wxSerialPort_SetBaudRate(*args, **kwargs)
    def SetLineState(*args, **kwargs): return _wxctb.wxSerialPort_SetLineState(*args, **kwargs)
    def Write(*args, **kwargs): return _wxctb.wxSerialPort_Write(*args, **kwargs)
_wxctb.wxSerialPort_swigregister(wxSerialPort)


GetKey = _wxctb.GetKey


