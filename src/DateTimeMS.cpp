#include "DateTimeMS.h"
DateTimeMSClass DateTimeMS;

  time_t DateTimeMSClass::osTimeMS(unsigned int &ms) {

    struct timeval tp;
    void *tzp=0;
//    auto ret = _gettimeofday_r(&unused, &tp, tzp);

    gettimeofday(&tp,tzp);
    ms = tp.tv_usec/1000;
    return tp.tv_sec;
  };


// -----------------------------------------------------------------
// get Time String with fake ms (000)
// -----------------------------------------------------------------
String getStringTimeWithMS() {
  unsigned int ms;
  String strTime = "";
  strTime += DateTimeMS.osTimeMS(ms);
  String strms;
  strms += ms;
  for (int i=0;3-strms.length();i++) {
    strms = "0"+strms;
  }
  return strTime+strms;
}