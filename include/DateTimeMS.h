#ifndef MY_DATE_TIME_CLASS_H
#define MY_DATE_TIME_CLASS_H
#include <DateTime.h>

String getStringTimeWithMS();

/**
 * @brief DateTime Library Main Class, include time get/set/format methods.
 *
 */
class DateTimeMSClass: public DateTimeClass {
 public:

    /**
   * @brief Get os timestamp, in milli seconds
   *
   * @return time_t timestamp, in milli seconds
   */
  time_t osTimeMS(unsigned int &ms);
};

/**
 * @brief Global DateTimeClass object.
 *
 */
extern DateTimeMSClass DateTimeMS;  // define in cpp

#endif
