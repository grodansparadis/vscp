// mdf_dump.h
//
// Dump everything a parsed CMDF holds as "path = value" lines. Used by the
// round trip tests: an MDF loaded from XML, from JSON, or re-loaded after
// save_xml()/save_json() must give the same dump.

#pragma once

#include <mdf.h>

#include <map>
#include <sstream>
#include <string>

class MdfDump {
public:
  std::string dump(CMDF &mdf)
  {
    m_out.str("");
    module(mdf);
    return m_out.str();
  }

private:
  std::ostringstream m_out;

  template<typename T> void v(const std::string &path, const T &value) { m_out << path << " = " << value << "\n"; }
  void v(const std::string &path, uint8_t value) { m_out << path << " = " << (unsigned) value << "\n"; }

  void texts(const std::string &p, CMDF_Object *pobj)
  {
    std::map<std::string, std::string> *pmap;
    if ((pmap = pobj->getMapDescription())) {
      for (auto &kv : *pmap) {
        v(p + "/description[" + kv.first + "]", kv.second);
      }
    }
    if ((pmap = pobj->getMapInfoUrl())) {
      for (auto &kv : *pmap) {
        v(p + "/infourl[" + kv.first + "]", kv.second);
      }
    }
  }

  static std::string idx(const std::string &p, size_t i) { return p + "[" + std::to_string(i) + "]"; }

  void values(const std::string &p, std::deque<CMDF_Value *> *plist)
  {
    v(p + "/valuelist#", plist->size());
    for (size_t i = 0; i < plist->size(); i++) {
      CMDF_Value *pv     = (*plist)[i];
      std::string q = idx(p + "/value", i);
      v(q + "/name", pv->getName());
      v(q + "/value", pv->getValue());
      texts(q, pv);
    }
  }

  void bits(const std::string &p, std::deque<CMDF_Bit *> *plist)
  {
    v(p + "/bit#", plist->size());
    for (size_t i = 0; i < plist->size(); i++) {
      CMDF_Bit *pb  = (*plist)[i];
      std::string q = idx(p + "/bit", i);
      v(q + "/name", pb->getName());
      v(q + "/pos", pb->getPos());
      v(q + "/width", pb->getWidth());
      v(q + "/default", pb->getDefault());
      v(q + "/min", pb->getMin());
      v(q + "/max", pb->getMax());
      v(q + "/access", (int) pb->getAccess());
      v(q + "/mask", pb->getMask());
      texts(q, pb);
      values(q, pb->getListValues());
    }
  }

  void items(const std::string &p, std::deque<CMDF_Item *> *plist)
  {
    v(p + "#", plist->size());
    for (size_t i = 0; i < plist->size(); i++) {
      CMDF_Item *pi = (*plist)[i];
      std::string q = idx(p, i);
      v(q + "/value", pi->getValue());
      texts(q, pi);
    }
  }

  void module(CMDF &mdf)
  {
    v("redirect", mdf.getRedirectUrl());
    v("name", mdf.getModuleName());
    v("model", mdf.getModuleModel());
    v("version", mdf.getModuleVersion());
    v("changed", mdf.getModuleChangeDate());
    v("copyright", mdf.getModuleCopyright());
    v("level", mdf.getModuleLevel());
    v("buffersize", mdf.getModuleBufferSize());
    texts("module", &mdf);

    CMDF_Manufacturer *pman = mdf.getManufacturer();
    v("manufacturer/name", pman->getName());
    CMDF_Address *paddr = pman->getAddressObj();
    v("manufacturer/address/street", paddr->getStreet());
    v("manufacturer/address/town", paddr->getTown());
    v("manufacturer/address/city", paddr->getCity());
    v("manufacturer/address/postcode", paddr->getPostCode());
    v("manufacturer/address/state", paddr->getState());
    v("manufacturer/address/region", paddr->getRegion());
    v("manufacturer/address/country", paddr->getCountry());
    items("manufacturer/telephone", pman->getPhoneContactList());
    items("manufacturer/fax", pman->getFaxContactList());
    items("manufacturer/email", pman->getEmailContactList());
    items("manufacturer/web", pman->getWebContactList());
    items("manufacturer/social", pman->getSocialContactList());

    CMDF_BootLoaderInfo *pboot = mdf.getBootLoaderObj();
    v("boot/algorithm", pboot->getAlgorithm());
    v("boot/blocksize", pboot->getBlockSize());
    v("boot/blockcount", pboot->getBlockCount());

    files(mdf);
    registers(mdf);
    remotevars(mdf);
    bits("alarm", mdf.getAlarmList());
    dmatrix(mdf);
    events(mdf);
  }

  void files(CMDF &mdf)
  {
    auto *pics = mdf.getPictureObjList();
    v("picture#", pics->size());
    for (size_t i = 0; i < pics->size(); i++) {
      auto *p       = (*pics)[i];
      std::string q = idx("picture", i);
      v(q + "/name", p->getName());
      v(q + "/url", p->getUrl());
      v(q + "/format", p->getFormat());
      v(q + "/date", p->getDate());
      v(q + "/version", p->getVersion());
      texts(q, p);
    }
    auto *videos = mdf.getVideoObjList();
    v("video#", videos->size());
    for (size_t i = 0; i < videos->size(); i++) {
      auto *p       = (*videos)[i];
      std::string q = idx("video", i);
      v(q + "/name", p->getName());
      v(q + "/url", p->getUrl());
      v(q + "/format", p->getFormat());
      v(q + "/date", p->getDate());
      v(q + "/version", p->getVersion());
      texts(q, p);
    }
    auto *fws = mdf.getFirmwareObjList();
    v("firmware#", fws->size());
    for (size_t i = 0; i < fws->size(); i++) {
      auto *p       = (*fws)[i];
      std::string q = idx("firmware", i);
      v(q + "/name", p->getName());
      v(q + "/url", p->getUrl());
      v(q + "/target", p->getTarget());
      v(q + "/targetcode", p->getTargetCode());
      v(q + "/format", p->getFormat());
      v(q + "/date", p->getDate());
      v(q + "/version", p->getVersion());
      v(q + "/size", p->getSize());
      v(q + "/md5", p->getMd5());
      texts(q, p);
    }
    auto *drvs = mdf.getDriverObjList();
    v("driver#", drvs->size());
    for (size_t i = 0; i < drvs->size(); i++) {
      auto *p       = (*drvs)[i];
      std::string q = idx("driver", i);
      v(q + "/name", p->getName());
      v(q + "/url", p->getUrl());
      v(q + "/type", p->getType());
      v(q + "/architecture", p->getArchitecture());
      v(q + "/os", p->getOS());
      v(q + "/osver", p->getOSVer());
      v(q + "/version", p->getVersion());
      v(q + "/md5", p->getMd5());
      v(q + "/date", p->getDate());
      texts(q, p);
    }
    auto *setups = mdf.getSetupObjList();
    v("setup#", setups->size());
    for (size_t i = 0; i < setups->size(); i++) {
      auto *p       = (*setups)[i];
      std::string q = idx("setup", i);
      v(q + "/name", p->getName());
      v(q + "/url", p->getUrl());
      v(q + "/format", p->getFormat());
      v(q + "/date", p->getDate());
      v(q + "/version", p->getVersion());
      texts(q, p);
    }
    auto *mans = mdf.getManualObjList();
    v("manual#", mans->size());
    for (size_t i = 0; i < mans->size(); i++) {
      auto *p       = (*mans)[i];
      std::string q = idx("manual", i);
      v(q + "/name", p->getName());
      v(q + "/url", p->getUrl());
      v(q + "/format", p->getFormat());
      v(q + "/lang", p->getLanguage());
      v(q + "/date", p->getDate());
      v(q + "/version", p->getVersion());
      texts(q, p);
    }
  }

  void registers(CMDF &mdf)
  {
    auto *regs = mdf.getRegisterObjList();
    v("register#", regs->size());
    for (size_t i = 0; i < regs->size(); i++) {
      CMDF_Register *pr = (*regs)[i];
      std::string q     = idx("register", i);
      v(q + "/name", pr->getName());
      v(q + "/page", pr->getPage());
      v(q + "/offset", pr->getOffset());
      v(q + "/span", pr->getSpan());
      v(q + "/width", pr->getWidth());
      v(q + "/type", (int) pr->getType());
      v(q + "/min", pr->getMin());
      v(q + "/max", pr->getMax());
      v(q + "/default", pr->getDefault());
      v(q + "/access", (int) pr->getAccess());
      v(q + "/fgcolor", pr->getForegroundColor());
      v(q + "/bgcolor", pr->getBackgroundColor());
      v(q + "/fgeven", pr->getForegroundColorEven());
      v(q + "/fgodd", pr->getForegroundColorOdd());
      v(q + "/bgeven", pr->getBackgroundColorEven());
      v(q + "/bgodd", pr->getBackgroundColorOdd());
      v(q + "/fgcolor-mdf", pr->getForegroundColorMdf());
      v(q + "/bgcolor-mdf", pr->getBackgroundColorMdf());
      texts(q, pr);
      bits(q, pr->getListBits());
      values(q, pr->getListValues());
    }
  }

  void remotevars(CMDF &mdf)
  {
    auto *rvars = mdf.getRemoteVariableList();
    v("remotevar#", rvars->size());
    for (size_t i = 0; i < rvars->size(); i++) {
      CMDF_RemoteVariable *pr = (*rvars)[i];
      std::string q           = idx("remotevar", i);
      v(q + "/name", pr->getName());
      v(q + "/type", (int) pr->getType());
      v(q + "/default", pr->getDefault());
      v(q + "/page", pr->getPage());
      v(q + "/offset", pr->getOffset());
      v(q + "/bitpos", pr->getBitPos());
      v(q + "/size", pr->getStringSize());
      v(q + "/min", pr->getMin());
      v(q + "/max", pr->getMax());
      v(q + "/access", (int) pr->getAccess());
      v(q + "/fgcolor", pr->getForegroundColor());
      v(q + "/bgcolor", pr->getBackgroundColor());
      texts(q, pr);
      bits(q, pr->getListBits());
      values(q, pr->getListValues());
    }
  }

  void dmatrix(CMDF &mdf)
  {
    CMDF_DecisionMatrix *pdm = mdf.getDM();
    v("dmatrix/level", pdm->getLevel());
    v("dmatrix/start-page", pdm->getStartPage());
    v("dmatrix/start-offset", pdm->getStartOffset());
    v("dmatrix/rowcount", pdm->getRowCount());
    v("dmatrix/rowsize", pdm->getRowSize());
    auto *actions = pdm->getActionList();
    v("dmatrix/action#", actions->size());
    for (size_t i = 0; i < actions->size(); i++) {
      CMDF_Action *pa = (*actions)[i];
      std::string q   = idx("dmatrix/action", i);
      v(q + "/name", pa->getName());
      v(q + "/code", pa->getCode());
      texts(q, pa);
      auto *params = pa->getListActionParameter();
      v(q + "/param#", params->size());
      for (size_t j = 0; j < params->size(); j++) {
        CMDF_ActionParameter *pp = (*params)[j];
        std::string r            = idx(q + "/param", j);
        v(r + "/name", pp->getName());
        v(r + "/offset", pp->getOffset());
        v(r + "/min", pp->getMin());
        v(r + "/max", pp->getMax());
        texts(r, pp);
        bits(r, pp->getListBits());
        values(r, pp->getListValues());
      }
    }
  }

  void events(CMDF &mdf)
  {
    auto *evs = mdf.getEventList();
    v("event#", evs->size());
    for (size_t i = 0; i < evs->size(); i++) {
      CMDF_Event *pe = (*evs)[i];
      std::string q  = idx("event", i);
      v(q + "/name", pe->getName());
      v(q + "/class", pe->getClass());
      v(q + "/type", pe->getType());
      v(q + "/priority", pe->getPriority());
      v(q + "/direction", (int) pe->getDirection());
      texts(q, pe);
      auto *datas = pe->getListEventData();
      v(q + "/data#", datas->size());
      for (size_t j = 0; j < datas->size(); j++) {
        CMDF_EventData *pd = (*datas)[j];
        std::string r      = idx(q + "/data", j);
        v(r + "/name", pd->getName());
        v(r + "/offset", pd->getOffset());
        texts(r, pd);
        bits(r, pd->getListBits());
        values(r, pd->getListValues());
      }
    }
  }
};
