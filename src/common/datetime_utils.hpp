/*
 *****************************************************************
 *                    Date Time Parsing Utils                    *
 *                                                               *
 * Author: Arash Partow (2006)                                   *
 * URL: http://www.partow.net/programming/datetime/index.html    *
 *                                                               *
 * Note:                                                         *
 * The following code has a dependency on the StrTk library.     *
 * URL: http://www.partow.net/programming/strtk/index.html       *
 *                                                               *
 * Copyright notice:                                             *
 * Free use of the Date Time Parsing Utils Library is permitted  *
 * under the guidelines and in accordance with the most current  *
 * version of the Common Public License.                         *
 * http://www.opensource.org/licenses/cpl1.0.php                 *
 *                                                               *
 *****************************************************************
*/


#ifndef INCLUDE_DATETIME_UTILS_HPP
#define INCLUDE_DATETIME_UTILS_HPP


#include <iostream>
#include <string>

#include "strtk.hpp"


namespace dt_utils
{
   struct datetime
   {
      unsigned short year;
      unsigned short month;
      unsigned short day;
      unsigned short hour;
      unsigned short minute;
      unsigned short second;
      unsigned short millisecond;
               short tzd; //as minutes.

      void clear()
      {
         year        = 0;
         month       = 0;
         day         = 0;
         hour        = 0;
         minute      = 0;
         second      = 0;
         millisecond = 0;
         tzd         = 0;
      }
   };

   /* YYYYMMDD    */ struct date_format00 { date_format00(datetime& d) : dt(d) {} datetime& dt; };
   /* YYYYDDMM    */ struct date_format01 { date_format01(datetime& d) : dt(d) {} datetime& dt; };
   /* YYYY/MM/DD  */ struct date_format02 { date_format02(datetime& d) : dt(d) {} datetime& dt; };
   /* YYYY/DD/MM  */ struct date_format03 { date_format03(datetime& d) : dt(d) {} datetime& dt; };
   /* DD/MM/YYYY  */ struct date_format04 { date_format04(datetime& d) : dt(d) {} datetime& dt; };
   /* MM/DD/YYYY  */ struct date_format05 { date_format05(datetime& d) : dt(d) {} datetime& dt; };
   /* YYYY-MM-DD  */ struct date_format06 { date_format06(datetime& d) : dt(d) {} datetime& dt; };
   /* YYYY-DD-MM  */ struct date_format07 { date_format07(datetime& d) : dt(d) {} datetime& dt; };
   /* DD-MM-YYYY  */ struct date_format08 { date_format08(datetime& d) : dt(d) {} datetime& dt; };
   /* MM-DD-YYYY  */ struct date_format09 { date_format09(datetime& d) : dt(d) {} datetime& dt; };
   /* DD.MM.YYYY  */ struct date_format10 { date_format10(datetime& d) : dt(d) {} datetime& dt; };
   /* MM.DD.YYYY  */ struct date_format11 { date_format11(datetime& d) : dt(d) {} datetime& dt; };
   /* DD-Mon-YY   */ struct date_format12 { date_format12(datetime& d) : dt(d) {} datetime& dt; };
   /* ?D-Mon-YY   */ struct date_format13 { date_format13(datetime& d) : dt(d) {} datetime& dt; };
   /* DD-Mon-YYYY */ struct date_format14 { date_format14(datetime& d) : dt(d) {} datetime& dt; };
   /* ?D-Mon-YYYY */ struct date_format15 { date_format15(datetime& d) : dt(d) {} datetime& dt; };

   /* HH:MM:SS.mss */ struct time_format0 { time_format0(datetime& d) : dt(d) {} datetime& dt; };
   /* HH:MM:SS     */ struct time_format1 { time_format1(datetime& d) : dt(d) {} datetime& dt; };
   /* HH MM SS mss */ struct time_format2 { time_format2(datetime& d) : dt(d) {} datetime& dt; };
   /* HH MM SS     */ struct time_format3 { time_format3(datetime& d) : dt(d) {} datetime& dt; };
   /* HH.MM.SS.mss */ struct time_format4 { time_format4(datetime& d) : dt(d) {} datetime& dt; };
   /* HH.MM.SS     */ struct time_format5 { time_format5(datetime& d) : dt(d) {} datetime& dt; };

   /* YYYYMMDD HH:MM:SS.mss    */ struct datetime_format00 { datetime_format00(datetime& d) : dt(d) {} datetime& dt; };
   /* YYYY/MM/DD HH:MM:SS.mss  */ struct datetime_format01 { datetime_format01(datetime& d) : dt(d) {} datetime& dt; };
   /* DD/MM/YYYY HH:MM:SS.mss  */ struct datetime_format02 { datetime_format02(datetime& d) : dt(d) {} datetime& dt; };
   /* YYYYMMDD HH:MM:SS        */ struct datetime_format03 { datetime_format03(datetime& d) : dt(d) {} datetime& dt; };
   /* YYYY/MM/DD HH:MM:SS      */ struct datetime_format04 { datetime_format04(datetime& d) : dt(d) {} datetime& dt; };
   /* DD/MM/YYYY HH:MM:SS      */ struct datetime_format05 { datetime_format05(datetime& d) : dt(d) {} datetime& dt; };
   /* YYYY-MM-DD HH:MM:SS.mss  */ struct datetime_format06 { datetime_format06(datetime& d) : dt(d) {} datetime& dt; };
   /* DD-MM-YYYY HH:MM:SS.mss  */ struct datetime_format07 { datetime_format07(datetime& d) : dt(d) {} datetime& dt; };
   /* YYYY-MM-DD HH:MM:SS      */ struct datetime_format08 { datetime_format08(datetime& d) : dt(d) {} datetime& dt; };
   /* DD-MM-YYYY HH:MM:SS      */ struct datetime_format09 { datetime_format09(datetime& d) : dt(d) {} datetime& dt; };
   /* YYYY-MM-DDTHH:MM:SS      */ struct datetime_format10 { datetime_format10(datetime& d) : dt(d) {} datetime& dt; };
   /* YYYY-MM-DDTHH:MM:SS.mss  */ struct datetime_format11 { datetime_format11(datetime& d) : dt(d) {} datetime& dt; };
   /* YYYYMMDDTHH:MM:SS        */ struct datetime_format12 { datetime_format12(datetime& d) : dt(d) {} datetime& dt; };
   /* YYYYMMDDTHH:MM:SS.mss    */ struct datetime_format13 { datetime_format13(datetime& d) : dt(d) {} datetime& dt; };
   /* DD-MM-YYYYTHH:MM:SS.mss  */ struct datetime_format14 { datetime_format14(datetime& d) : dt(d) {} datetime& dt; };
   /* DD-MM-YYYYTHH:MM:SS      */ struct datetime_format15 { datetime_format15(datetime& d) : dt(d) {} datetime& dt; };
   /* YYYYMMDDTHHMM            */ struct datetime_format16 { datetime_format16(datetime& d) : dt(d) {} datetime& dt; };
   /* YYYYMMDDTHHMMSS          */ struct datetime_format17 { datetime_format17(datetime& d) : dt(d) {} datetime& dt; };
   /* YYYYMMDDTHHMMSSMSS       */ struct datetime_format18 { datetime_format18(datetime& d) : dt(d) {} datetime& dt; };
   /* ISO8601 DateThh:mm:ssTZD */ struct datetime_format19 { datetime_format19(datetime& d) : dt(d) {} datetime& dt; };
   /* ISO8601 DateThh:mmTZD    */ struct datetime_format20 { datetime_format20(datetime& d) : dt(d) {} datetime& dt; };
   /* NCSA Common Log DateTime */ struct datetime_format21 { datetime_format21(datetime& d) : dt(d) {} datetime& dt; };
   /* RFC-822 HTTP DateTime    */ struct datetime_format22 { datetime_format22(datetime& d) : dt(d) {} datetime& dt; };

   class dt_format
   {
   public:

      virtual ~dt_format() {}

      virtual bool process(const char* begin, const char* end) = 0;
   };

   namespace details
   {
      template <typename InputIterator>
      bool parse_YYYYMMDD(InputIterator begin, InputIterator end, datetime& dt)
      {
         const std::size_t date_size = 8;
         unsigned int date;
         if (date_size != static_cast<std::size_t>(std::distance(begin,end)))
            return false;
         else if (!strtk::fast::all_digits_check<8>(begin))
            return false;
         strtk::fast::numeric_convert<8>(begin,date);
         if (date < 101)
            return false;
         dt.year  = date / 10000;
         dt.month = (date % 10000) / 100;
         dt.day   = date % 100;
         return true;
      }

      template <typename InputIterator>
      bool parse_YYYYDDMM(InputIterator begin, InputIterator end, datetime& dt)
      {
         const std::size_t date_size = 8;
         unsigned int date;
         if (date_size != static_cast<std::size_t>(std::distance(begin,end)))
            return false;
         else if (!strtk::fast::all_digits_check<8>(begin))
            return false;
         strtk::fast::numeric_convert<8>(begin,date);
         if (date < 101)
            return false;
         dt.year  = date / 10000;
         dt.month = date % 100;
         dt.day   = (date % 10000) / 100;
         return true;
      }

      template <typename InputIterator>
      unsigned int dow3chr_to_index(const InputIterator dow)
      {
         /* Sun: 1, Mon: 2, Tue: 3, Wed: 4, Thu: 5, Fri: 6, Sat: 7 */
         const unsigned int error = 0;
         switch (std::toupper(dow[0]))
         {
            case 'M' : return (std::toupper(dow[1]) == 'O' && std::toupper(dow[2]) == 'N') ?  2 : error;
            case 'W' : return (std::toupper(dow[1]) == 'E' && std::toupper(dow[2]) == 'D') ?  4 : error;
            case 'F' : return (std::toupper(dow[1]) == 'R' && std::toupper(dow[2]) == 'I') ?  6 : error;
            case 'T' :
                      {
                         char c0 = std::toupper(dow[1]);
                         char c1 = std::toupper(dow[2]);
                         if (('U' == c0) && (c1 == 'E'))
                            return 3;
                         else if (('H' == c0) && (c1 == 'U'))
                            return 5;
                         else
                            return error;
                      }
            case 'S' :
                      {
                         char c0 = std::toupper(dow[1]);
                         char c1 = std::toupper(dow[2]);
                         if (('A' == c0) && (c1 == 'T'))
                            return 7;
                         else if (('U' == c0) && (c1 == 'N'))
                            return 1;
                         else
                            return error;
                      }
         }
         return false;
      }

      template <typename InputIterator>
      unsigned int month3chr_to_index(const InputIterator month)
      {
         const unsigned int error = 0;
         switch (std::toupper(month[0]))
         {
            case 'F' : return (std::toupper(month[1]) == 'E' && std::toupper(month[2]) == 'B') ?  2 : error; //February
            case 'D' : return (std::toupper(month[1]) == 'E' && std::toupper(month[2]) == 'C') ? 12 : error; //December
            case 'N' : return (std::toupper(month[1]) == 'O' && std::toupper(month[2]) == 'V') ? 11 : error; //November
            case 'O' : return (std::toupper(month[1]) == 'C' && std::toupper(month[2]) == 'T') ? 10 : error; //October
            case 'S' : return (std::toupper(month[1]) == 'E' && std::toupper(month[2]) == 'P') ?  9 : error; //September
            case 'A' :
                      {
                         char c0 = std::toupper(month[1]);
                         char c1 = std::toupper(month[2]);
                         if (('P' == c0) && (c1 == 'R'))      //April
                            return 4;
                         else if (('U' == c0) && (c1 == 'G')) //August
                            return 8;
                         else
                            return error;
                      }

            case 'J' :
                      {
                         char c0 = std::toupper(month[1]);
                         char c1 = std::toupper(month[2]);
                         if (('A' == c0) && (c1 == 'N'))      //January
                            return 1;
                         else if (('U' == c0) && (c1 == 'L')) //July
                            return 7;
                         else if (('U' == c0) && (c1 == 'N')) //June
                            return 6;
                         else
                            return error;
                      }

            case 'M' :
                      {
                         char c0 = std::toupper(month[1]);
                         char c1 = std::toupper(month[2]);
                         if (('A' == c0) && (c1 == 'R'))      //March
                            return 3;
                         else if (('A' == c0) && (c1 == 'Y')) //May
                            return 5;
                         else
                            return error;
                      }

             default : return error;
         }
      }

      template <typename InputIterator>
      bool tzd3chr_to_offset(const InputIterator tzd, short& offset_mins)
      {
         if ('T' != std::toupper(tzd[2]))
            return false;
         else if ('D' == std::toupper(tzd[1]))
         {
            switch (std::toupper(tzd[0]))
            {
               case 'E' : offset_mins = -4; break;
               case 'C' : offset_mins = -5; break;
               case 'M' : offset_mins = -6; break;
               case 'P' : offset_mins = -7; break;
               default  : return false;
            }
         }
         else if ('S' == std::toupper(tzd[1]))
         {
            switch (std::toupper(tzd[0]))
            {
               case 'E' : offset_mins = -5; break;
               case 'C' : offset_mins = -6; break;
               case 'M' : offset_mins = -7; break;
               case 'P' : offset_mins = -8; break;
               default  : return false;
            }
         }
         else if (('G' == std::toupper(tzd[0])) && ('M' == std::toupper(tzd[1])))
            offset_mins = 0;
         else
            return false;
         offset_mins *= 60;
         return true;
      }

      template <typename InputIterator>
      bool miltzd1chr_to_offset(const InputIterator tzd, short& offset_mins)
      {
         switch (std::toupper(tzd[0]))
         {
            case 'A' : offset_mins = -1;  break;
            case 'M' : offset_mins = -12; break;
            case 'N' : offset_mins = +1;  break;
            case 'Y' : offset_mins = +12; break;
            default  : return false;
         }
         offset_mins *= 60;
         return true;
      }

      #define register_datetime_format_proxy(Type)            \
      struct Type##_proxy : public dt_format                  \
      {                                                       \
         Type##_proxy(datetime& dt)                           \
         : dtf_(dt)                                           \
         {}                                                   \
         Type dtf_;                                           \
         bool process(const char* b, const char* e)           \
         {                                                    \
            return strtk::string_to_type_converter(b,e,dtf_); \
         }                                                    \
      };                                                      \

      register_datetime_format_proxy(date_format00)
      register_datetime_format_proxy(date_format01)
      register_datetime_format_proxy(date_format02)
      register_datetime_format_proxy(date_format03)
      register_datetime_format_proxy(date_format04)
      register_datetime_format_proxy(date_format05)
      register_datetime_format_proxy(date_format06)
      register_datetime_format_proxy(date_format07)
      register_datetime_format_proxy(date_format08)
      register_datetime_format_proxy(date_format09)
      register_datetime_format_proxy(date_format10)
      register_datetime_format_proxy(date_format11)
      register_datetime_format_proxy(date_format12)
      register_datetime_format_proxy(date_format13)
      register_datetime_format_proxy(date_format14)
      register_datetime_format_proxy(date_format15)
      register_datetime_format_proxy(time_format0)
      register_datetime_format_proxy(time_format1)
      register_datetime_format_proxy(time_format2)
      register_datetime_format_proxy(time_format3)
      register_datetime_format_proxy(time_format4)
      register_datetime_format_proxy(time_format5)
      register_datetime_format_proxy(datetime_format00)
      register_datetime_format_proxy(datetime_format01)
      register_datetime_format_proxy(datetime_format02)
      register_datetime_format_proxy(datetime_format03)
      register_datetime_format_proxy(datetime_format04)
      register_datetime_format_proxy(datetime_format05)
      register_datetime_format_proxy(datetime_format06)
      register_datetime_format_proxy(datetime_format07)
      register_datetime_format_proxy(datetime_format08)
      register_datetime_format_proxy(datetime_format09)
      register_datetime_format_proxy(datetime_format10)
      register_datetime_format_proxy(datetime_format11)
      register_datetime_format_proxy(datetime_format12)
      register_datetime_format_proxy(datetime_format13)
      register_datetime_format_proxy(datetime_format14)
      register_datetime_format_proxy(datetime_format15)
      register_datetime_format_proxy(datetime_format16)
      register_datetime_format_proxy(datetime_format17)
      register_datetime_format_proxy(datetime_format18)
      register_datetime_format_proxy(datetime_format19)
      register_datetime_format_proxy(datetime_format20)
      register_datetime_format_proxy(datetime_format21)
      register_datetime_format_proxy(datetime_format22)
   }

   bool valid_date00(const datetime& dt)
   {
      if ((dt.day < 1) || (dt.day > 31))
         return false;
      else if ((dt.month < 1) || (dt.month > 12))
         return false;
      else
         return true;
   }

   bool valid_date01(const datetime& dt)
   {
      if ((dt.day < 1) || (dt.day > 31))
         return false;
      else if ((dt.month < 1) || (dt.month > 12))
         return false;

      static const unsigned int days_in_month[] =
                            {
                              0,
                              31, 28, 31, 30,
                              31, 30, 31, 31,
                              30, 31, 30, 31
                            };

      if (dt.month == 2)
      {
         struct is_leap_year
         {
            static inline bool process(const unsigned int y)
            {
               return (0 == (y % 4)) && ((0 != (y % 100)) || (0 == (y % 400)));
            }
         };

         if (!is_leap_year::process(dt.year))
            return (dt.day <= days_in_month[dt.month]);
         else
            return (dt.day <= 29);
      }
      else
         return (dt.day <= days_in_month[dt.month]);
   }

   bool valid_time00(const datetime& dt)
   {
      if (dt.hour > 23)
         return false;
      else if (dt.minute > 59)
         return false;
      else if (dt.second > 59)
         return false;
      else if (dt.millisecond > 999)
         return false;
      else
         return true;
   }

   bool valid_datetime00(const datetime& dt)
   {
      return valid_date00(dt) && valid_time00(dt);
   }

   bool valid_datetime01(const datetime& dt)
   {
      return valid_date01(dt) && valid_time00(dt);
   }

   void test()
   {
      {
         std::string data = "20060317";
         dt_utils::datetime dt;
         dt_utils::date_format00 dtd00(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dtd00)) std::cout << "ERROR - date_format00 YYYYMMDD\n";
         if (dt.year  != 2006) std::cout << "ERROR - date_format00 - year \n";
         if (dt.month !=    3) std::cout << "ERROR - date_format00 - month\n";
         if (dt.day   !=   17) std::cout << "ERROR - date_format00 - day  \n";
      }

      {
         std::string data = "20061703";
         dt_utils::datetime dt;
         dt_utils::date_format01 dtd1(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dtd1)) std::cout << "ERROR - date_format01 YYYYDDMM\n";
         if (dt.year  != 2006) std::cout << "ERROR - date_format01 - year \n";
         if (dt.month !=    3) std::cout << "ERROR - date_format01 - month\n";
         if (dt.day   !=   17) std::cout << "ERROR - date_format01 - day  \n";
      }

      {
         std::string data = "2006/03/17";
         dt_utils::datetime dt;
         dt_utils::date_format02 dtd2(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dtd2)) std::cout << "ERROR - date_format02 YYYY/MM/DD\n";
         if (dt.year  != 2006) std::cout << "ERROR - date_format02 - year \n";
         if (dt.month !=    3) std::cout << "ERROR - date_format02 - month\n";
         if (dt.day   !=   17) std::cout << "ERROR - date_format02 - day  \n";
      }

      {
         std::string data = "2006/17/03";
         dt_utils::datetime dt;
         dt_utils::date_format03 dtd3(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dtd3)) std::cout << "ERROR - date_format03 YYYY/DD/MM\n";
         if (dt.year  != 2006) std::cout << "ERROR - date_format03 - year \n";
         if (dt.month !=    3) std::cout << "ERROR - date_format03 - month\n";
         if (dt.day   !=   17) std::cout << "ERROR - date_format03 - day  \n";
      }

      {
         std::string data = "17/03/2006";
         dt_utils::datetime dt;
         dt_utils::date_format04 dtd4(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dtd4)) std::cout << "ERROR - date_format04 DD/MM/YYYY\n";
         if (dt.year  != 2006) std::cout << "ERROR - date_format04 - year \n";
         if (dt.month !=    3) std::cout << "ERROR - date_format04 - month\n";
         if (dt.day   !=   17) std::cout << "ERROR - date_format04 - day  \n";
      }

      {
         std::string data = "03/17/2006";
         dt_utils::datetime dt;
         dt_utils::date_format05 dtd5(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dtd5)) std::cout << "ERROR - date_format05 MM/DD/YYYY\n";
         if (dt.year  != 2006) std::cout << "ERROR - date_format05 - year \n";
         if (dt.month !=    3) std::cout << "ERROR - date_format05 - month\n";
         if (dt.day   !=   17) std::cout << "ERROR - date_format05 - day  \n";
      }

      {
         std::string data = "2006-03-17";
         dt_utils::datetime dt;
         dt_utils::date_format06 dtd6(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dtd6)) std::cout << "ERROR - date_format06 YYYY-MM-DD\n";
         if (dt.year  != 2006) std::cout << "ERROR - date_format06 - year \n";
         if (dt.month !=    3) std::cout << "ERROR - date_format06 - month\n";
         if (dt.day   !=   17) std::cout << "ERROR - date_format06 - day  \n";
      }

      {
         std::string data = "2006-17-03";
         dt_utils::datetime dt;
         dt_utils::date_format07 dtd7(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dtd7)) std::cout << "ERROR - date_format07 YYYY-DD-MM\n";
         if (dt.year  != 2006) std::cout << "ERROR - date_format07 - year \n";
         if (dt.month !=    3) std::cout << "ERROR - date_format07 - month\n";
         if (dt.day   !=   17) std::cout << "ERROR - date_format07 - day  \n";
      }

      {
         std::string data = "17-03-2006";
         dt_utils::datetime dt;
         dt_utils::date_format08 dtd8(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dtd8)) std::cout << "ERROR - date_format08 DD-MM-YYYY\n";
         if (dt.year  != 2006) std::cout << "ERROR - date_format08 - year \n";
         if (dt.month !=    3) std::cout << "ERROR - date_format08 - month\n";
         if (dt.day   !=   17) std::cout << "ERROR - date_format08 - day  \n";
      }

      {
         std::string data = "03-17-2006";
         dt_utils::datetime dt;
         dt_utils::date_format09 dtd9(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dtd9)) std::cout << "ERROR - date_format09 MM-DD-YYYY\n";
         if (dt.year  != 2006) std::cout << "ERROR - date_format09 - year \n";
         if (dt.month !=    3) std::cout << "ERROR - date_format09 - month\n";
         if (dt.day   !=   17) std::cout << "ERROR - date_format09 - day  \n";
      }

      {
         std::string data = "17.03.2006";
         dt_utils::datetime dt;
         dt_utils::date_format10 dtd10(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dtd10)) std::cout << "ERROR - date_format10 DD.MM.YYYY\n";
         if (dt.year  != 2006) std::cout << "ERROR - date_format08 - year \n";
         if (dt.month !=    3) std::cout << "ERROR - date_format08 - month\n";
         if (dt.day   !=   17) std::cout << "ERROR - date_format08 - day  \n";
      }

      {
         std::string data = "03.17.2006";
         dt_utils::datetime dt;
         dt_utils::date_format11 dtd11(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dtd11)) std::cout << "ERROR - date_format11 MM.DD.YYYY\n";
         if (dt.year  != 2006) std::cout << "ERROR - date_format11 - year \n";
         if (dt.month !=    3) std::cout << "ERROR - date_format11 - month\n";
         if (dt.day   !=   17) std::cout << "ERROR - date_format11 - day  \n";
      }

      {
         std::string data = "17-Mar-06";
         dt_utils::datetime dt;
         dt_utils::date_format12 dtd12(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dtd12)) std::cout << "ERROR - date_format12 DD-Mon-YY\n";
         if (dt.year  !=  6) std::cout << "ERROR - date_format12 - year \n";
         if (dt.month !=  3) std::cout << "ERROR - date_format12 - month\n";
         if (dt.day   != 17) std::cout << "ERROR - date_format12 - day  \n";
      }

      {
         std::string data = "17-Mar-06";
         dt_utils::datetime dt;
         dt_utils::date_format13 dtd13(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dtd13)) std::cout << "ERROR - date_format13 ?D-Mon-YY\n";
         if (dt.year  !=  6) std::cout << "ERROR - date_format13 - year \n";
         if (dt.month !=  3) std::cout << "ERROR - date_format13 - month\n";
         if (dt.day   != 17) std::cout << "ERROR - date_format13 - day  \n";
      }

      {
         std::string data = "3-Mar-06";
         dt_utils::datetime dt;
         dt_utils::date_format13 dtd13(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dtd13)) std::cout << "ERROR - date_format13 ?D-Mon-YY\n";
         if (dt.year  != 6) std::cout << "ERROR - date_format13 - year \n";
         if (dt.month != 3) std::cout << "ERROR - date_format13 - month\n";
         if (dt.day   != 3) std::cout << "ERROR - date_format13 - day  \n";
      }

      {
         std::string data = "17-Mar-2006";
         dt_utils::datetime dt;
         dt_utils::date_format14 dtd14(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dtd14)) std::cout << "ERROR - date_format14 DD-Mon-YYYY\n";
         if (dt.year  != 2006) std::cout << "ERROR - date_format14 - year \n";
         if (dt.month !=    3) std::cout << "ERROR - date_format14 - month\n";
         if (dt.day   !=   17) std::cout << "ERROR - date_format14 - day  \n";
      }

      {
         std::string data = "17-Mar-2006";
         dt_utils::datetime dt;
         dt_utils::date_format15 dtd15(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dtd15)) std::cout << "ERROR - date_format15 ?D-Mon-YYYY\n";
         if (dt.year  != 2006) std::cout << "ERROR - date_format15 - year \n";
         if (dt.month !=    3) std::cout << "ERROR - date_format15 - month\n";
         if (dt.day   !=   17) std::cout << "ERROR - date_format15 - day  \n";
      }

      {
         std::string data = "3-Mar-2006";
         dt_utils::datetime dt;
         dt_utils::date_format15 dtd15(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dtd15)) std::cout << "ERROR - date_format15 ?D-Mon-YYYY\n";
         if (dt.year  != 2006) std::cout << "ERROR - date_format15 - year \n";
         if (dt.month !=    3) std::cout << "ERROR - date_format15 - month\n";
         if (dt.day   !=    3) std::cout << "ERROR - date_format15 - day  \n";
      }

      {
         std::string data = "13:27:54.123";
         dt_utils::datetime dt;
         dt_utils::time_format0 dtt0(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dtt0)) std::cout << "ERROR - time_format0 HH:MM:SS.MSS\n";
         if (dt.hour        !=  13) std::cout << "ERROR - time_format0 - hour       \n";
         if (dt.minute      !=  27) std::cout << "ERROR - time_format0 - minute     \n";
         if (dt.second      !=  54) std::cout << "ERROR - time_format0 - second     \n";
         if (dt.millisecond != 123) std::cout << "ERROR - time_format0 - millisecond\n";
      }

      {
         std::string data = "13:27:54";
         dt_utils::datetime dt;
         dt_utils::time_format1 dtt1(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dtt1)) std::cout << "ERROR - time_format1 HH:MM:SS\n";
         if (dt.hour        !=  13) std::cout << "ERROR - time_format1 - hour       \n";
         if (dt.minute      !=  27) std::cout << "ERROR - time_format1 - minute     \n";
         if (dt.second      !=  54) std::cout << "ERROR - time_format1 - second     \n";
      }

      {
         std::string data = "13 27 54 123";
         dt_utils::datetime dt;
         dt_utils::time_format2 dtt2(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dtt2)) std::cout << "ERROR - time_format2 HH MM SS MSS\n";
         if (dt.hour        !=  13) std::cout << "ERROR - time_format2 - hour       \n";
         if (dt.minute      !=  27) std::cout << "ERROR - time_format2 - minute     \n";
         if (dt.second      !=  54) std::cout << "ERROR - time_format2 - second     \n";
         if (dt.millisecond != 123) std::cout << "ERROR - time_format2 - millisecond\n";
      }

      {
         std::string data = "13 27 54";
         dt_utils::datetime dt;
         dt_utils::time_format3 dtt3(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dtt3)) std::cout << "ERROR - time_format3 HH MM SS\n";
         if (dt.hour        !=  13) std::cout << "ERROR - time_format3 - hour  \n";
         if (dt.minute      !=  27) std::cout << "ERROR - time_format3 - minute\n";
         if (dt.second      !=  54) std::cout << "ERROR - time_format3 - second\n";
      }

      {
         std::string data = "13.27.54.123";
         dt_utils::datetime dt;
         dt_utils::time_format4 dtt4(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dtt4)) std::cout << "ERROR - time_format4 HH.MM.SS.MSS\n";
         if (dt.hour        !=  13) std::cout << "ERROR - time_format4 - hour       \n";
         if (dt.minute      !=  27) std::cout << "ERROR - time_format4 - minute     \n";
         if (dt.second      !=  54) std::cout << "ERROR - time_format4 - second     \n";
         if (dt.millisecond != 123) std::cout << "ERROR - time_format4 - millisecond\n";
      }

      {
         std::string data = "13.27.54";
         dt_utils::datetime dt;
         dt_utils::time_format5 dtt5(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dtt5)) std::cout << "ERROR - time_format5 HH.MM.SS\n";
         if (dt.hour        !=  13) std::cout << "ERROR - time_format5 - hour  \n";
         if (dt.minute      !=  27) std::cout << "ERROR - time_format5 - minute\n";
         if (dt.second      !=  54) std::cout << "ERROR - time_format5 - second\n";
      }

      {
         std::string data = "20060317 13:27:54.123";
         dt_utils::datetime dt;
         dt_utils::datetime_format00 dt0(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt0)) std::cout << "ERROR - datetime_format00 YYYYMMDD HH:MM:SS.MSS\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format00 - year       \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format00 - month      \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format00 - day        \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format00 - hour       \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format00 - minute     \n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format00 - second     \n";
         if (dt.millisecond !=  123) std::cout << "ERROR - datetime_format00 - millisecond\n";
      }

      {
         std::string data = "2006/03/17 13:27:54.123";
         dt_utils::datetime dt;
         dt_utils::datetime_format01 dt1(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt1)) std::cout << "ERROR - datetime_format01 YYYY/MM/DD HH:MM:SS.MSS\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format01 - year       \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format01 - month      \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format01 - day        \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format01 - hour       \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format01 - minute     \n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format01 - second     \n";
         if (dt.millisecond !=  123) std::cout << "ERROR - datetime_format01 - millisecond\n";
      }

      {
         std::string data = "17/03/2006 13:27:54.123";
         dt_utils::datetime dt;
         dt_utils::datetime_format02 dt2(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt2)) std::cout << "ERROR - datetime_format02 DD/MM/YYYY HH:MM:SS.MSS\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format02 - year       \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format02 - month      \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format02 - day        \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format02 - hour       \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format02 - minute     \n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format02 - second     \n";
         if (dt.millisecond !=  123) std::cout << "ERROR - datetime_format02 - millisecond\n";
      }

      {
         std::string data = "20060317 13:27:54";
         dt_utils::datetime dt;
         dt_utils::datetime_format03 dt3(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt3)) std::cout << "ERROR - datetime_format03 YYYYMMDD HH:MM:SS\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format03 - year  \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format03 - month \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format03 - day   \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format03 - hour  \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format03 - minute\n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format03 - second\n";
      }

      {
         std::string data = "2006/03/17 13:27:54";
         dt_utils::datetime dt;
         dt_utils::datetime_format04 dt4(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt4)) std::cout << "ERROR - datetime_format04 YYYY/MM/DD HH:MM:SS\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format04 - year  \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format04 - month \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format04 - day   \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format04 - hour  \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format04 - minute\n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format04 - second\n";
      }

      {
         std::string data = "17/03/2006 13:27:54";
         dt_utils::datetime dt;
         dt_utils::datetime_format05 dt5(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt5)) std::cout << "ERROR - datetime_format05 DD/MM/YYYY HH:MM:SS\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format05 - year  \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format05 - month \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format05 - day   \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format05 - hour  \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format05 - minute\n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format05 - second\n";
      }

      {
         std::string data = "2006-03-17 13:27:54.123";
         dt_utils::datetime dt;
         dt_utils::datetime_format06 dt6(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt6)) std::cout << "ERROR - datetime_format06 YYYY-MM-DD HH:MM:SS.MSS\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format06 - year       \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format06 - month      \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format06 - day        \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format06 - hour       \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format06 - minute     \n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format06 - second     \n";
         if (dt.millisecond !=  123) std::cout << "ERROR - datetime_format06 - millisecond\n";
      }

      {
         std::string data = "17-03-2006 13:27:54.123";
         dt_utils::datetime dt;
         dt_utils::datetime_format07 dt7(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt7)) std::cout << "ERROR - datetime_format07 DD-MM-YYYY HH:MM:SS.MSS\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format07 - year       \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format07 - month      \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format07 - day        \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format07 - hour       \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format07 - minute     \n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format07 - second     \n";
         if (dt.millisecond !=  123) std::cout << "ERROR - datetime_format07 - millisecond\n";
      }

      {
         std::string data = "2006-03-17 13:27:54";
         dt_utils::datetime dt;
         dt_utils::datetime_format08 dt8(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt8)) std::cout << "ERROR - datetime_format08 YYYY-MM-DD HH:MM:SS\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format08 - year  \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format08 - month \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format08 - day   \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format08 - hour  \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format08 - minute\n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format08 - second\n";
      }

      {
         std::string data = "17-03-2006 13:27:54";
         dt_utils::datetime dt;
         dt_utils::datetime_format09 dt9(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt9)) std::cout << "ERROR - datetime_format09 DD-MM-YYYY HH:MM:SS\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format09 - year  \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format09 - month \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format09 - day   \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format09 - hour  \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format09 - minute\n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format09 - second\n";
      }

      {
         std::string data = "2006-03-17T13:27:54";
         dt_utils::datetime dt;
         dt_utils::datetime_format10 dt10(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt10)) std::cout << "ERROR - datetime_format10 YYYY-MM-DDTHH:MM:SS\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format10 - year  \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format10 - month \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format10 - day   \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format10 - hour  \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format10 - minute\n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format10 - second\n";
      }

      {
         std::string data = "2006-03-17T13:27:54.123";
         dt_utils::datetime dt;
         dt_utils::datetime_format11 dt11(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt11)) std::cout << "ERROR - datetime_format11 YYYY-MM-DDTHH:MM:SS.MSS\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format11 - year       \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format11 - month      \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format11 - day        \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format11 - hour       \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format11 - minute     \n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format11 - second     \n";
         if (dt.millisecond !=  123) std::cout << "ERROR - datetime_format11 - millisecond\n";
      }

      {
         std::string data = "20060317T13:27:54";
         dt_utils::datetime dt;
         dt_utils::datetime_format12 dt12(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt12)) std::cout << "ERROR - datetime_format12 YYYYMMDDTHH:MM:SS\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format12 - year  \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format12 - month \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format12 - day   \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format12 - hour  \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format12 - minute\n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format12 - second\n";
      }

      {
         std::string data = "20060317T13:27:54.123";
         dt_utils::datetime dt;
         dt_utils::datetime_format13 dt13(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt13)) std::cout << "ERROR - datetime_format13 YYYYMMDDTHH:MM:SS.MSS\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format13 - year       \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format13 - month      \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format13 - day        \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format13 - hour       \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format13 - minute     \n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format13 - second     \n";
         if (dt.millisecond !=  123) std::cout << "ERROR - datetime_format13 - millisecond\n";
      }

      {
         std::string data = "17-03-2006T13:27:54.123";
         dt_utils::datetime dt;
         dt_utils::datetime_format14 dt14(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt14)) std::cout << "ERROR - datetime_format14 DD-MM-YYYYTHH:MM:SS.MSS\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format14 - year       \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format14 - month      \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format14 - day        \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format14 - hour       \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format14 - minute     \n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format14 - second     \n";
         if (dt.millisecond !=  123) std::cout << "ERROR - datetime_format14 - millisecond\n";
      }

      {
         std::string data = "17-03-2006T13:27:54";
         dt_utils::datetime dt;
         dt_utils::datetime_format15 dt15(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt15)) std::cout << "ERROR - datetime_format15 DD-MM-YYYYTHH:MM:SS\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format15 - year  \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format15 - month \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format15 - day   \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format15 - hour  \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format15 - minute\n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format15 - second\n";
      }

      {
         std::string data = "20060317T1327";
         dt_utils::datetime dt;
         dt_utils::datetime_format16 dt16(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt16)) std::cout << "ERROR - datetime_format16 YYYYMMDDTHHMM\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format16 - year  \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format16 - month \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format16 - day   \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format16 - hour  \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format16 - minute\n";
      }

      {
         std::string data = "20060317T132754";
         dt_utils::datetime dt;
         dt_utils::datetime_format17 dt17(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt17)) std::cout << "ERROR - datetime_format17 YYYYMMDDTHHMM\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format17 - year  \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format17 - month \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format17 - day   \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format17 - hour  \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format17 - minute\n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format17 - second\n";
      }

      {
         std::string data = "20060317T132754123";
         dt_utils::datetime dt;
         dt_utils::datetime_format18 dt18(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt18)) std::cout << "ERROR - datetime_format18 YYYYMMDDTHHMMMSS\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format18 - year  \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format18 - month \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format18 - day   \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format18 - hour  \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format18 - minute\n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format18 - second\n";
         if (dt.millisecond !=  123) std::cout << "ERROR - datetime_format18 - millisecond\n";
      }

      {
         std::string data = "2006-03-17T13:27:54Z";
         dt_utils::datetime dt;
         dt_utils::datetime_format19 dt19(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt19)) std::cout << "ERROR - datetime_format19 ISO8601 (0)\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format19(0) - year  \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format19(0) - month \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format19(0) - day   \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format19(0) - hour  \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format19(0) - minute\n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format19(0) - second\n";
         if (dt.tzd         !=    0) std::cout << "ERROR - datetime_format19(0) - tzd   \n";
      }

      {
         std::string data = "2006-03-17T13:27:54+03:45";
         dt_utils::datetime dt;
         dt_utils::datetime_format19 dt19(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt19)) std::cout << "ERROR - datetime_format19 ISO8601 (1)n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format19(1) - year  \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format19(1) - month \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format19(1) - day   \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format19(1) - hour  \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format19(1) - minute\n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format19(1) - second\n";
         if (dt.tzd         !=  225) std::cout << "ERROR - datetime_format19(1) - tzd   \n";
      }

      {
         std::string data = "2006-03-17T13:27:54-05:37";
         dt_utils::datetime dt;
         dt_utils::datetime_format19 dt19(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt19)) std::cout << "ERROR - datetime_format19 ISO8601 (2)\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format19(2) - year  \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format19(2) - month \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format19(2) - day   \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format19(2) - hour  \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format19(2) - minute\n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format19(2) - second\n";
         if (dt.tzd         != -337) std::cout << "ERROR - datetime_format19(2) - tzd   \n";
      }

      {
         std::string data = "2006-03-17T13:27Z";
         dt_utils::datetime dt;
         dt_utils::datetime_format20 dt20(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt20)) std::cout << "ERROR - datetime_format20 ISO8601 (0)\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format20(0) - year  \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format20(0) - month \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format20(0) - day   \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format20(0) - hour  \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format20(0) - minute\n";
         if (dt.tzd         !=    0) std::cout << "ERROR - datetime_format20(0) - tzd   \n";
      }

      {
         std::string data = "2006-03-17T13:27+03:45";
         dt_utils::datetime dt;
         dt_utils::datetime_format20 dt20(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt20)) std::cout << "ERROR - datetime_format20 ISO8601 (1)\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format20(1) - year  \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format20(1) - month \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format20(1) - day   \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format20(1) - hour  \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format20(1) - minute\n";
         if (dt.tzd         !=  225) std::cout << "ERROR - datetime_format20(1) - tzd   \n";
      }

      {
         std::string data = "2006-03-17T13:27-05:37";
         dt_utils::datetime dt;
         dt_utils::datetime_format20 dt20(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt20)) std::cout << "ERROR - datetime_format20 ISO8601 (2)\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format20(2) - year  \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format20(2) - month \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format20(2) - day   \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format20(2) - hour  \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format20(2) - minute\n";
         if (dt.tzd         != -337) std::cout << "ERROR - datetime_format20(2) - tzd   \n";
      }

      {
         std::string data = "17/Mar/2006:13:27:54 -0537";
         dt_utils::datetime dt;
         dt_utils::datetime_format21 dt21(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt21)) std::cout << "ERROR - datetime_format21 NCSA CL (0)\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format21(0) - year  \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format21(0) - month \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format21(0) - day   \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format21(0) - hour  \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format21(0) - minute\n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format21(0) - second\n";
         if (dt.tzd         != -337) std::cout << "ERROR - datetime_format21(0) - tzd   \n";
      }

      {
         std::string data = "17/Mar/2006:13:27:54 +0537";
         dt_utils::datetime dt;
         dt_utils::datetime_format21 dt21(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt21)) std::cout << "ERROR - datetime_format21 NCSA CL (0)\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format21(0) - year  \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format21(0) - month \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format21(0) - day   \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format21(0) - hour  \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format21(0) - minute\n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format21(0) - second\n";
         if (dt.tzd         != +337) std::cout << "ERROR - datetime_format21(0) - tzd   \n";
      }

      {
         std::string data = "Sat, 17 Mar 2006 13:27:54 GMT";
         dt_utils::datetime dt;
         dt_utils::datetime_format22 dt22(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt22)) std::cout << "ERROR - datetime_format22 RFC-822(0)\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format22(0) - year  \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format22(0) - month \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format22(0) - day   \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format22(0) - hour  \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format22(0) - minute\n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format22(0) - second\n";
         if (dt.tzd         !=    0) std::cout << "ERROR - datetime_format22(0) - tzd   \n";
      }

      {
         std::string data = "Sat, 17 Mar 2006 13:27:54 EST";
         dt_utils::datetime dt;
         dt_utils::datetime_format22 dt22(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt22)) std::cout << "ERROR - datetime_format22 RFC-822(1)\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format22(1) - year  \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format22(1) - month \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format22(1) - day   \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format22(1) - hour  \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format22(1) - minute\n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format22(1) - second\n";
         if (dt.tzd         != -300) std::cout << "ERROR - datetime_format22(1) - tzd   \n";
      }

      {
         std::string data = "Sat, 17 Mar 2006 13:27:54 UT";
         dt_utils::datetime dt;
         dt_utils::datetime_format22 dt22(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt22)) std::cout << "ERROR - datetime_format22 RFC-822(2)\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format22(2) - year  \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format22(2) - month \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format22(2) - day   \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format22(2) - hour  \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format22(2) - minute\n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format22(2) - second\n";
         if (dt.tzd         !=    0) std::cout << "ERROR - datetime_format22(2) - tzd   \n";
      }

      {
         std::string data = "Sat, 17 Mar 2006 13:27:54 M";
         dt_utils::datetime dt;
         dt_utils::datetime_format22 dt22(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt22)) std::cout << "ERROR - datetime_format22 RFC-822(3)\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format22(3) - year  \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format22(3) - month \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format22(3) - day   \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format22(3) - hour  \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format22(3) - minute\n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format22(3) - second\n";
         if (dt.tzd         != -720) std::cout << "ERROR - datetime_format22(3) - tzd   \n";
      }

      {
         std::string data = "Sat, 17 Mar 2006 13:27:54 -0234";
         dt_utils::datetime dt;
         dt_utils::datetime_format22 dt22(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt22)) std::cout << "ERROR - datetime_format22 RFC-822(4)\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format22(4) - year  \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format22(4) - month \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format22(4) - day   \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format22(4) - hour  \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format22(4) - minute\n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format22(4) - second\n";
         if (dt.tzd         != -154) std::cout << "ERROR - datetime_format22(4) - tzd   \n";
      }

      {
         std::string data = "Sat, 17 Mar 2006 13:27:54 +0325";
         dt_utils::datetime dt;
         dt_utils::datetime_format22 dt22(dt);
         dt.clear();
         if (!strtk::string_to_type_converter(data,dt22)) std::cout << "ERROR - datetime_format22 RFC-822(5)\n";
         if (dt.year        != 2006) std::cout << "ERROR - datetime_format22(5) - year  \n";
         if (dt.month       !=    3) std::cout << "ERROR - datetime_format22(5) - month \n";
         if (dt.day         !=   17) std::cout << "ERROR - datetime_format22(5) - day   \n";
         if (dt.hour        !=   13) std::cout << "ERROR - datetime_format22(5) - hour  \n";
         if (dt.minute      !=   27) std::cout << "ERROR - datetime_format22(5) - minute\n";
         if (dt.second      !=   54) std::cout << "ERROR - datetime_format22(5) - second\n";
         if (dt.tzd         != +205) std::cout << "ERROR - datetime_format22(5) - tzd   \n";
      }
   }

   dt_format* create_datetime(const std::string& format, datetime& dt)
   {
           if (format == "YYYYMMDD"               ) return new details::date_format00_proxy    (dt);
      else if (format == "YYYYDDMM"               ) return new details::date_format01_proxy    (dt);
      else if (format == "YYYY/MM/DD"             ) return new details::date_format02_proxy    (dt);
      else if (format == "YYYY/DD/MM"             ) return new details::date_format03_proxy    (dt);
      else if (format == "DD/MM/YYYY"             ) return new details::date_format04_proxy    (dt);
      else if (format == "MM/DD/YYYY"             ) return new details::date_format05_proxy    (dt);
      else if (format == "YYYY-MM-DD"             ) return new details::date_format06_proxy    (dt);
      else if (format == "YYYY-DD-MM"             ) return new details::date_format07_proxy    (dt);
      else if (format == "DD-MM-YYYY"             ) return new details::date_format08_proxy    (dt);
      else if (format == "MM-DD-YYYY"             ) return new details::date_format09_proxy    (dt);
      else if (format == "DD.MM.YYYY"             ) return new details::date_format10_proxy    (dt);
      else if (format == "MM.DD.YYYY"             ) return new details::date_format11_proxy    (dt);
      else if (format == "DD-Mon-YY"              ) return new details::date_format12_proxy    (dt);
      else if (format == "?D-Mon-YY"              ) return new details::date_format13_proxy    (dt);
      else if (format == "DD-Mon-YYYY"            ) return new details::date_format14_proxy    (dt);
      else if (format == "?D-Mon-YYYY"            ) return new details::date_format15_proxy    (dt);
      else if (format == "HH:MM:SS.mss"           ) return new details::time_format0_proxy     (dt);
      else if (format == "HH:MM:SS"               ) return new details::time_format1_proxy     (dt);
      else if (format == "HH MM SS mss"           ) return new details::time_format2_proxy     (dt);
      else if (format == "HH MM SS"               ) return new details::time_format3_proxy     (dt);
      else if (format == "HH.MM.SS.mss"           ) return new details::time_format4_proxy     (dt);
      else if (format == "HH.MM.SS"               ) return new details::time_format5_proxy     (dt);
      else if (format == "YYYYMMDD HH:MM:SS.mss"  ) return new details::datetime_format00_proxy(dt);
      else if (format == "YYYY/MM/DD HH:MM:SS.mss") return new details::datetime_format01_proxy(dt);
      else if (format == "DD/MM/YYYY HH:MM:SS.mss") return new details::datetime_format02_proxy(dt);
      else if (format == "YYYYMMDD HH:MM:SS"      ) return new details::datetime_format03_proxy(dt);
      else if (format == "YYYY/MM/DD HH:MM:SS"    ) return new details::datetime_format04_proxy(dt);
      else if (format == "DD/MM/YYYY HH:MM:SS"    ) return new details::datetime_format05_proxy(dt);
      else if (format == "YYYY-MM-DD HH:MM:SS.mss") return new details::datetime_format06_proxy(dt);
      else if (format == "DD-MM-YYYY HH:MM:SS.mss") return new details::datetime_format07_proxy(dt);
      else if (format == "YYYY-MM-DD HH:MM:SS"    ) return new details::datetime_format08_proxy(dt);
      else if (format == "DD-MM-YYYY HH:MM:SS"    ) return new details::datetime_format09_proxy(dt);
      else if (format == "YYYY-MM-DDTHH:MM:SS"    ) return new details::datetime_format10_proxy(dt);
      else if (format == "YYYY-MM-DDTHH:MM:SS.mss") return new details::datetime_format11_proxy(dt);
      else if (format == "YYYYMMDDTHH:MM:SS"      ) return new details::datetime_format12_proxy(dt);
      else if (format == "YYYYMMDDTHH:MM:SS.mss"  ) return new details::datetime_format13_proxy(dt);
      else if (format == "DD-MM-YYYYTHH:MM:SS.mss") return new details::datetime_format14_proxy(dt);
      else if (format == "DD-MM-YYYYTHH:MM:SS"    ) return new details::datetime_format15_proxy(dt);
      else if (format == "YYYYMMDDTHHMM"          ) return new details::datetime_format16_proxy(dt);
      else if (format == "YYYYMMDDTHHMMSS"        ) return new details::datetime_format17_proxy(dt);
      else if (format == "YYYYMMDDTHHMMSSMSS"     ) return new details::datetime_format18_proxy(dt);
      else if (format == "ISO8601-0"              ) return new details::datetime_format19_proxy(dt);
      else if (format == "ISO8601-1"              ) return new details::datetime_format20_proxy(dt);
      else if (format == "CommonLog"              ) return new details::datetime_format21_proxy(dt);
      else if (format == "RFC822"                 ) return new details::datetime_format22_proxy(dt);
      else                                          return reinterpret_cast<dt_format*>(0);
   }
}

strtk_string_to_type_begin(dt_utils::dt_format*)
   return t->process(begin,end);
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::date_format00)
   if (8 != std::distance(begin,end))
      return false;
   else
     return dt_utils::details::parse_YYYYMMDD(begin,begin + 8,t.dt);
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::date_format01)
   if (8 != std::distance(begin,end))
      return false;
   else
      return dt_utils::details::parse_YYYYDDMM(begin,begin + 8,t.dt);
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::date_format02)
   if (10 != std::distance(begin,end))
      return false;
   else if (('/' != *(begin + 4)) || ('/' != *(begin + 7)))
      return false;
   else if (
             !strtk::fast::all_digits_check<4>(begin + 0) ||
             !strtk::fast::all_digits_check<2>(begin + 5) ||
             !strtk::fast::all_digits_check<2>(begin + 8)
           )
      return false;
   strtk::fast::numeric_convert<4>(begin + 0,t.dt.year );
   strtk::fast::numeric_convert<2>(begin + 5,t.dt.month);
   strtk::fast::numeric_convert<2>(begin + 8,t.dt.day  );
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::date_format03)
   if (10 != std::distance(begin,end))
      return false;
   else if (('/' != *(begin + 4)) || ('/' != *(begin + 7)))
      return false;
   else if (
             !strtk::fast::all_digits_check<4>(begin + 0) ||
             !strtk::fast::all_digits_check<2>(begin + 5) ||
             !strtk::fast::all_digits_check<2>(begin + 8)
           )
      return false;
   strtk::fast::numeric_convert<4>(begin + 0,t.dt.year );
   strtk::fast::numeric_convert<2>(begin + 5,t.dt.day  );
   strtk::fast::numeric_convert<2>(begin + 8,t.dt.month);
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::date_format04)
   if (10 != std::distance(begin,end))
      return false;
   else if (('/' != *(begin + 2)) || ('/' != *(begin + 5)))
      return false;
   else if (
             !strtk::fast::all_digits_check<2>(begin + 0) ||
             !strtk::fast::all_digits_check<2>(begin + 3) ||
             !strtk::fast::all_digits_check<4>(begin + 6)
           )
      return false;
   strtk::fast::numeric_convert<2>(begin + 0,t.dt.day  );
   strtk::fast::numeric_convert<2>(begin + 3,t.dt.month);
   strtk::fast::numeric_convert<4>(begin + 6,t.dt.year );
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::date_format05)
   if (10 != std::distance(begin,end))
      return false;
   else if (('/' != *(begin + 2)) || ('/' != *(begin + 5)))
      return false;
   else if (
             !strtk::fast::all_digits_check<2>(begin + 0) ||
             !strtk::fast::all_digits_check<2>(begin + 3) ||
             !strtk::fast::all_digits_check<4>(begin + 6)
           )
      return false;
   strtk::fast::numeric_convert<2>(begin + 0,t.dt.month);
   strtk::fast::numeric_convert<2>(begin + 3,t.dt.day  );
   strtk::fast::numeric_convert<4>(begin + 6,t.dt.year );
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::date_format06)
   if (10 != std::distance(begin,end))
      return false;
   else if (('-' != *(begin + 4)) || ('-' != *(begin + 7)))
      return false;
   else if (
             !strtk::fast::all_digits_check<4>(begin + 0) ||
             !strtk::fast::all_digits_check<2>(begin + 5) ||
             !strtk::fast::all_digits_check<2>(begin + 8)
           )
      return false;
   strtk::fast::numeric_convert<4>(begin + 0,t.dt.year );
   strtk::fast::numeric_convert<2>(begin + 5,t.dt.month);
   strtk::fast::numeric_convert<2>(begin + 8,t.dt.day  );
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::date_format07)
   if (10 != std::distance(begin,end))
      return false;
   else if (('-' != *(begin + 4)) || ('-' != *(begin + 7)))
      return false;
   else if (
             !strtk::fast::all_digits_check<4>(begin + 0) ||
             !strtk::fast::all_digits_check<2>(begin + 5) ||
             !strtk::fast::all_digits_check<2>(begin + 8)
           )
      return false;
   strtk::fast::numeric_convert<4>(begin + 0,t.dt.year );
   strtk::fast::numeric_convert<2>(begin + 5,t.dt.day  );
   strtk::fast::numeric_convert<2>(begin + 8,t.dt.month);
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::date_format08)
   if (10 != std::distance(begin,end))
      return false;
   else if (('-' != *(begin + 2)) || ('-' != *(begin + 5)))
      return false;
   else if (
             !strtk::fast::all_digits_check<2>(begin + 0) ||
             !strtk::fast::all_digits_check<2>(begin + 3) ||
             !strtk::fast::all_digits_check<4>(begin + 6)
           )
      return false;
   strtk::fast::numeric_convert<2>(begin + 0,t.dt.day  );
   strtk::fast::numeric_convert<2>(begin + 3,t.dt.month);
   strtk::fast::numeric_convert<4>(begin + 6,t.dt.year );
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::date_format09)
   if (10 != std::distance(begin,end))
      return false;
   else if (('-' != *(begin + 2)) || ('-' != *(begin + 5)))
      return false;
   else if (
             !strtk::fast::all_digits_check<2>(begin + 0) ||
             !strtk::fast::all_digits_check<2>(begin + 3) ||
             !strtk::fast::all_digits_check<4>(begin + 6)
           )
      return false;
   strtk::fast::numeric_convert<2>(begin + 0,t.dt.month);
   strtk::fast::numeric_convert<2>(begin + 3,t.dt.day  );
   strtk::fast::numeric_convert<4>(begin + 6,t.dt.year );
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::date_format10)
   if (10 != std::distance(begin,end))
      return false;
   else if (('.' != *(begin + 2)) || ('.' != *(begin + 5)))
      return false;
   else if (
             !strtk::fast::all_digits_check<2>(begin + 0) ||
             !strtk::fast::all_digits_check<2>(begin + 3) ||
             !strtk::fast::all_digits_check<4>(begin + 6)
           )
      return false;
   strtk::fast::numeric_convert<2>(begin + 0,t.dt.day  );
   strtk::fast::numeric_convert<2>(begin + 3,t.dt.month);
   strtk::fast::numeric_convert<4>(begin + 6,t.dt.year );
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::date_format11)
   if (10 != std::distance(begin,end))
      return false;
   else if (('.' != *(begin + 2)) || ('.' != *(begin + 5)))
      return false;
   else if (
             !strtk::fast::all_digits_check<2>(begin + 0) ||
             !strtk::fast::all_digits_check<2>(begin + 3) ||
             !strtk::fast::all_digits_check<4>(begin + 6)
           )
      return false;
   strtk::fast::numeric_convert<2>(begin + 0,t.dt.month);
   strtk::fast::numeric_convert<2>(begin + 3,t.dt.day  );
   strtk::fast::numeric_convert<4>(begin + 6,t.dt.year );
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::date_format12)
   if (9 != std::distance(begin,end))
      return false;
   else if (('-' != *(begin + 2)) || ('-' != *(begin + 6)))
      return false;
   else if (
             !strtk::fast::all_digits_check<2>(begin + 0) ||
             !strtk::fast::all_digits_check<2>(begin + 7)
           )
      return false;
   strtk::fast::numeric_convert<2>(begin + 0,t.dt.day );
   strtk::fast::numeric_convert<2>(begin + 7,t.dt.year);
   return (0 != (t.dt.month = dt_utils::details::month3chr_to_index(begin + 3)));
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::date_format13)
   const std::size_t length = std::distance(begin,end);
   if (9 == length)
   {
      if (('-' != *(begin + 2)) || ('-' != *(begin + 6)))
         return false;
      else if (
                !strtk::fast::all_digits_check<2>(begin + 0) ||
                !strtk::fast::all_digits_check<2>(begin + 7)
              )
         return false;
      strtk::fast::numeric_convert<2>(begin + 0,t.dt.day );
      strtk::fast::numeric_convert<2>(begin + 7,t.dt.year);
      return (0 != (t.dt.month = dt_utils::details::month3chr_to_index(begin + 3)));
   }
   else if (8 == length)
   {
      if (('-' != *(begin + 1)) || ('-' != *(begin + 5)))
         return false;
      else if (
                !strtk::fast::all_digits_check<1>(begin + 0) ||
                !strtk::fast::all_digits_check<2>(begin + 6)
              )
         return false;
      strtk::fast::numeric_convert<1>(begin + 0,t.dt.day );
      strtk::fast::numeric_convert<2>(begin + 6,t.dt.year);
      return (0 != (t.dt.month = dt_utils::details::month3chr_to_index(begin + 2)));
   }
   else
      return false;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::date_format14)
   if (11 != std::distance(begin,end))
      return false;
   else if (('-' != *(begin + 2)) || ('-' != *(begin + 6)))
      return false;
   else if (
             !strtk::fast::all_digits_check<2>(begin + 0) ||
             !strtk::fast::all_digits_check<4>(begin + 7)
           )
      return false;
   strtk::fast::numeric_convert<2>(begin + 0,t.dt.day );
   strtk::fast::numeric_convert<4>(begin + 7,t.dt.year);
   return (0 != (t.dt.month = dt_utils::details::month3chr_to_index(begin + 3)));
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::date_format15)
   const std::size_t length = std::distance(begin,end);
   if (11 == length)
   {
      if (('-' != *(begin + 2)) || ('-' != *(begin + 6)))
         return false;
      else if (
                !strtk::fast::all_digits_check<2>(begin + 0) ||
                !strtk::fast::all_digits_check<4>(begin + 7)
              )
         return false;
      strtk::fast::numeric_convert<2>(begin + 0,t.dt.day );
      strtk::fast::numeric_convert<4>(begin + 7,t.dt.year);
      return (0 != (t.dt.month = dt_utils::details::month3chr_to_index(begin + 3)));
   }
   else if (10 == length)
   {
      if (('-' != *(begin + 1)) || ('-' != *(begin + 5)))
         return false;
      else if (
                !strtk::fast::all_digits_check<1>(begin + 0) ||
                !strtk::fast::all_digits_check<4>(begin + 6)
              )
         return false;
      strtk::fast::numeric_convert<1>(begin + 0,t.dt.day );
      strtk::fast::numeric_convert<4>(begin + 6,t.dt.year);
      return (0 != (t.dt.month = dt_utils::details::month3chr_to_index(begin + 2)));
   }
   else
      return false;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::time_format0)
   if (12 != std::distance(begin,end))
      return false;
   else if (
             (':' != *(begin + 2)) ||
             (':' != *(begin + 5)) ||
             ('.' != *(begin + 8))
           )
      return false;
   else if (
             !strtk::fast::all_digits_check<2>(begin + 0) ||
             !strtk::fast::all_digits_check<2>(begin + 3) ||
             !strtk::fast::all_digits_check<2>(begin + 6) ||
             !strtk::fast::all_digits_check<3>(begin + 9)
           )
      return false;
   strtk::fast::numeric_convert<2>(begin + 0,t.dt.hour       );
   strtk::fast::numeric_convert<2>(begin + 3,t.dt.minute     );
   strtk::fast::numeric_convert<2>(begin + 6,t.dt.second     );
   strtk::fast::numeric_convert<3>(begin + 9,t.dt.millisecond);
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::time_format1)
   if (8 != std::distance(begin,end))
      return false;
   else if ((':' != *(begin + 2)) || (':' != *(begin + 5)))
      return false;
   else if (
             !strtk::fast::all_digits_check<2>(begin + 0) ||
             !strtk::fast::all_digits_check<2>(begin + 3) ||
             !strtk::fast::all_digits_check<2>(begin + 6)
           )
      return false;
   strtk::fast::numeric_convert<2>(begin + 0,t.dt.hour  );
   strtk::fast::numeric_convert<2>(begin + 3,t.dt.minute);
   strtk::fast::numeric_convert<2>(begin + 6,t.dt.second);
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::time_format2)
   if (12 != std::distance(begin,end))
      return false;
   else if (
             (' ' != *(begin + 2)) ||
             (' ' != *(begin + 5)) ||
             (' ' != *(begin + 8))
           )
      return false;
   else if (
             !strtk::fast::all_digits_check<2>(begin + 0) ||
             !strtk::fast::all_digits_check<2>(begin + 3) ||
             !strtk::fast::all_digits_check<2>(begin + 6) ||
             !strtk::fast::all_digits_check<3>(begin + 9)
           )
      return false;
   strtk::fast::numeric_convert<2>(begin + 0,t.dt.hour       );
   strtk::fast::numeric_convert<2>(begin + 3,t.dt.minute     );
   strtk::fast::numeric_convert<2>(begin + 6,t.dt.second     );
   strtk::fast::numeric_convert<3>(begin + 9,t.dt.millisecond);
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::time_format3)
   if (8 != std::distance(begin,end))
      return false;
   else if ((' ' != *(begin + 2)) || (' ' != *(begin + 5)))
      return false;
   else if (
             !strtk::fast::all_digits_check<2>(begin + 0) ||
             !strtk::fast::all_digits_check<2>(begin + 3) ||
             !strtk::fast::all_digits_check<2>(begin + 6)
           )
      return false;
   strtk::fast::numeric_convert<2>(begin + 0,t.dt.hour  );
   strtk::fast::numeric_convert<2>(begin + 3,t.dt.minute);
   strtk::fast::numeric_convert<2>(begin + 6,t.dt.second);
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::time_format4)
   if (12 != std::distance(begin,end))
      return false;
   else if (
             ('.' != *(begin + 2)) ||
             ('.' != *(begin + 5)) ||
             ('.' != *(begin + 8))
           )
      return false;
   else if (
             !strtk::fast::all_digits_check<2>(begin + 0) ||
             !strtk::fast::all_digits_check<2>(begin + 3) ||
             !strtk::fast::all_digits_check<2>(begin + 6) ||
             !strtk::fast::all_digits_check<3>(begin + 9)
           )
      return false;
   strtk::fast::numeric_convert<2>(begin + 0,t.dt.hour       );
   strtk::fast::numeric_convert<2>(begin + 3,t.dt.minute     );
   strtk::fast::numeric_convert<2>(begin + 6,t.dt.second     );
   strtk::fast::numeric_convert<3>(begin + 9,t.dt.millisecond);
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::time_format5)
   if (8 != std::distance(begin,end))
      return false;
   else if (('.' != *(begin + 2)) || ('.' != *(begin + 5)))
      return false;
   else if (
             !strtk::fast::all_digits_check<2>(begin + 0) ||
             !strtk::fast::all_digits_check<2>(begin + 3) ||
             !strtk::fast::all_digits_check<2>(begin + 6)
           )
      return false;
   strtk::fast::numeric_convert<2>(begin + 0,t.dt.hour  );
   strtk::fast::numeric_convert<2>(begin + 3,t.dt.minute);
   strtk::fast::numeric_convert<2>(begin + 6,t.dt.second);
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::datetime_format00)
   if (21 != std::distance(begin,end))
      return false;
   else if (!dt_utils::details::parse_YYYYMMDD(begin,begin + 8,t.dt))
      return false;
   else if (
             (' ' != *(begin +  8)) || (':' != *(begin + 11)) ||
             (':' != *(begin + 14)) || ('.' != *(begin + 17))
           )
      return false;
   else if (
             !strtk::fast::all_digits_check<2>(begin +  9) ||
             !strtk::fast::all_digits_check<2>(begin + 12) ||
             !strtk::fast::all_digits_check<2>(begin + 15) ||
             !strtk::fast::all_digits_check<3>(begin + 18)
           )
      return false;
   strtk::fast::numeric_convert<2>(begin +  9,t.dt.hour       );
   strtk::fast::numeric_convert<2>(begin + 12,t.dt.minute     );
   strtk::fast::numeric_convert<2>(begin + 15,t.dt.second     );
   strtk::fast::numeric_convert<3>(begin + 18,t.dt.millisecond);
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::datetime_format01)
   if (23 != std::distance(begin,end))
      return false;
   else if (
             ('/' != *(begin +  4)) || ('/' != *(begin +  7)) ||
             (' ' != *(begin + 10)) || (':' != *(begin + 13)) ||
             (':' != *(begin + 16)) || ('.' != *(begin + 19))
           )
      return false;
   else if (
             !strtk::fast::all_digits_check<4>(begin +  0) ||
             !strtk::fast::all_digits_check<2>(begin +  5) ||
             !strtk::fast::all_digits_check<2>(begin +  8) ||
             !strtk::fast::all_digits_check<2>(begin + 11) ||
             !strtk::fast::all_digits_check<2>(begin + 14) ||
             !strtk::fast::all_digits_check<2>(begin + 17) ||
             !strtk::fast::all_digits_check<3>(begin + 20)
           )
      return false;
   strtk::fast::numeric_convert<4>(begin +  0,t.dt.year       );
   strtk::fast::numeric_convert<2>(begin +  5,t.dt.month      );
   strtk::fast::numeric_convert<2>(begin +  8,t.dt.day        );
   strtk::fast::numeric_convert<2>(begin + 11,t.dt.hour       );
   strtk::fast::numeric_convert<2>(begin + 14,t.dt.minute     );
   strtk::fast::numeric_convert<2>(begin + 17,t.dt.second     );
   strtk::fast::numeric_convert<3>(begin + 20,t.dt.millisecond);
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::datetime_format02)
   if (23 != std::distance(begin,end))
      return false;
   else if (
             ('/' != *(begin +  2)) || ('/' != *(begin +  5)) ||
             (' ' != *(begin + 10)) || (':' != *(begin + 13)) ||
             (':' != *(begin + 16)) || ('.' != *(begin + 19))
           )
      return false;
   else if (
             !strtk::fast::all_digits_check<2>(begin +  0) ||
             !strtk::fast::all_digits_check<2>(begin +  3) ||
             !strtk::fast::all_digits_check<4>(begin +  6) ||
             !strtk::fast::all_digits_check<2>(begin + 11) ||
             !strtk::fast::all_digits_check<2>(begin + 14) ||
             !strtk::fast::all_digits_check<2>(begin + 17) ||
             !strtk::fast::all_digits_check<3>(begin + 20)
           )
      return false;
   strtk::fast::numeric_convert<2>(begin +  0,t.dt.day        );
   strtk::fast::numeric_convert<2>(begin +  3,t.dt.month      );
   strtk::fast::numeric_convert<4>(begin +  6,t.dt.year       );
   strtk::fast::numeric_convert<2>(begin + 11,t.dt.hour       );
   strtk::fast::numeric_convert<2>(begin + 14,t.dt.minute     );
   strtk::fast::numeric_convert<2>(begin + 17,t.dt.second     );
   strtk::fast::numeric_convert<3>(begin + 20,t.dt.millisecond);
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::datetime_format03)
   if (17 != std::distance(begin,end))
      return false;
   else if (
             (' ' != *(begin +  8)) ||
             (':' != *(begin + 11)) ||
             (':' != *(begin + 14))
           )
      return false;
   else if (
             !strtk::fast::all_digits_check<4>(begin +  0) ||
             !strtk::fast::all_digits_check<2>(begin +  4) ||
             !strtk::fast::all_digits_check<2>(begin +  6) ||
             !strtk::fast::all_digits_check<2>(begin +  9) ||
             !strtk::fast::all_digits_check<2>(begin + 12) ||
             !strtk::fast::all_digits_check<2>(begin + 15)
           )
      return false;
   strtk::fast::numeric_convert<4>(begin +  0,t.dt.year  );
   strtk::fast::numeric_convert<2>(begin +  4,t.dt.month );
   strtk::fast::numeric_convert<2>(begin +  6,t.dt.day   );
   strtk::fast::numeric_convert<2>(begin +  9,t.dt.hour  );
   strtk::fast::numeric_convert<2>(begin + 12,t.dt.minute);
   strtk::fast::numeric_convert<2>(begin + 15,t.dt.second);
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::datetime_format04)
   if (19 != std::distance(begin,end))
      return false;
   else if (
             ('/' != *(begin +  4)) || ('/' != *(begin +  7)) ||
             (' ' != *(begin + 10)) || (':' != *(begin + 13)) ||
             (':' != *(begin + 16))
           )
      return false;
   else if (
             !strtk::fast::all_digits_check<4>(begin +  0) ||
             !strtk::fast::all_digits_check<2>(begin +  5) ||
             !strtk::fast::all_digits_check<2>(begin +  8) ||
             !strtk::fast::all_digits_check<2>(begin + 11) ||
             !strtk::fast::all_digits_check<2>(begin + 14) ||
             !strtk::fast::all_digits_check<2>(begin + 17)
           )
      return false;
   strtk::fast::numeric_convert<4>(begin +  0,t.dt.year  );
   strtk::fast::numeric_convert<2>(begin +  5,t.dt.month );
   strtk::fast::numeric_convert<2>(begin +  8,t.dt.day   );
   strtk::fast::numeric_convert<2>(begin + 11,t.dt.hour  );
   strtk::fast::numeric_convert<2>(begin + 14,t.dt.minute);
   strtk::fast::numeric_convert<2>(begin + 17,t.dt.second);
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::datetime_format05)
   if (19 != std::distance(begin,end))
      return false;
   else if (
             ('/' != *(begin +  2)) || ('/' != *(begin +  5)) ||
             (' ' != *(begin + 10)) || (':' != *(begin + 13)) ||
             (':' != *(begin + 16))
           )
      return false;
   else if (
             !strtk::fast::all_digits_check<2>(begin +  0) ||
             !strtk::fast::all_digits_check<2>(begin +  3) ||
             !strtk::fast::all_digits_check<4>(begin +  6) ||
             !strtk::fast::all_digits_check<2>(begin + 11) ||
             !strtk::fast::all_digits_check<2>(begin + 14) ||
             !strtk::fast::all_digits_check<2>(begin + 17)
           )
      return false;
   strtk::fast::numeric_convert<2>(begin +  0,t.dt.day   );
   strtk::fast::numeric_convert<2>(begin +  3,t.dt.month );
   strtk::fast::numeric_convert<4>(begin +  6,t.dt.year  );
   strtk::fast::numeric_convert<2>(begin + 11,t.dt.hour  );
   strtk::fast::numeric_convert<2>(begin + 14,t.dt.minute);
   strtk::fast::numeric_convert<2>(begin + 17,t.dt.second);
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::datetime_format06)
   if (23 != std::distance(begin,end))
      return false;
   else if (
             ('-' != *(begin +  4)) || ('-' != *(begin +  7)) ||
             (' ' != *(begin + 10)) || (':' != *(begin + 13)) ||
             (':' != *(begin + 16)) || ('.' != *(begin + 19))
           )
      return false;
   else if (
             !strtk::fast::all_digits_check<4>(begin +  0) ||
             !strtk::fast::all_digits_check<2>(begin +  5) ||
             !strtk::fast::all_digits_check<2>(begin +  8) ||
             !strtk::fast::all_digits_check<2>(begin + 11) ||
             !strtk::fast::all_digits_check<2>(begin + 14) ||
             !strtk::fast::all_digits_check<2>(begin + 17) ||
             !strtk::fast::all_digits_check<3>(begin + 20)
           )
      return false;
   strtk::fast::numeric_convert<4>(begin +  0,t.dt.year       );
   strtk::fast::numeric_convert<2>(begin +  5,t.dt.month      );
   strtk::fast::numeric_convert<2>(begin +  8,t.dt.day        );
   strtk::fast::numeric_convert<2>(begin + 11,t.dt.hour       );
   strtk::fast::numeric_convert<2>(begin + 14,t.dt.minute     );
   strtk::fast::numeric_convert<2>(begin + 17,t.dt.second     );
   strtk::fast::numeric_convert<3>(begin + 20,t.dt.millisecond);
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::datetime_format07)
   if (23 != std::distance(begin,end))
      return false;
   else if (
             ('-' != *(begin +  2)) || ('-' != *(begin +  5)) ||
             (' ' != *(begin + 10)) || (':' != *(begin + 13)) ||
             (':' != *(begin + 16)) || ('.' != *(begin + 19))
           )
      return false;
   else if (
             !strtk::fast::all_digits_check<2>(begin +  0) ||
             !strtk::fast::all_digits_check<2>(begin +  3) ||
             !strtk::fast::all_digits_check<4>(begin +  6) ||
             !strtk::fast::all_digits_check<2>(begin + 11) ||
             !strtk::fast::all_digits_check<2>(begin + 14) ||
             !strtk::fast::all_digits_check<2>(begin + 17) ||
             !strtk::fast::all_digits_check<3>(begin + 20)
           )
      return false;
   strtk::fast::numeric_convert<2>(begin +  0,t.dt.day        );
   strtk::fast::numeric_convert<2>(begin +  3,t.dt.month      );
   strtk::fast::numeric_convert<4>(begin +  6,t.dt.year       );
   strtk::fast::numeric_convert<2>(begin + 11,t.dt.hour       );
   strtk::fast::numeric_convert<2>(begin + 14,t.dt.minute     );
   strtk::fast::numeric_convert<2>(begin + 17,t.dt.second     );
   strtk::fast::numeric_convert<3>(begin + 20,t.dt.millisecond);
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::datetime_format08)
   if (19 != std::distance(begin,end))
      return false;
   else if (
             ('-' != *(begin +  4)) || ('-' != *(begin +  7)) ||
             (' ' != *(begin + 10)) || (':' != *(begin + 13)) ||
             (':' != *(begin + 16))
           )
      return false;
   else if (
             !strtk::fast::all_digits_check<4>(begin +  0) ||
             !strtk::fast::all_digits_check<2>(begin +  5) ||
             !strtk::fast::all_digits_check<2>(begin +  8) ||
             !strtk::fast::all_digits_check<2>(begin + 11) ||
             !strtk::fast::all_digits_check<2>(begin + 14) ||
             !strtk::fast::all_digits_check<2>(begin + 17)
           )
      return false;
   strtk::fast::numeric_convert<4>(begin +  0,t.dt.year  );
   strtk::fast::numeric_convert<2>(begin +  5,t.dt.month );
   strtk::fast::numeric_convert<2>(begin +  8,t.dt.day   );
   strtk::fast::numeric_convert<2>(begin + 11,t.dt.hour  );
   strtk::fast::numeric_convert<2>(begin + 14,t.dt.minute);
   strtk::fast::numeric_convert<2>(begin + 17,t.dt.second);
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::datetime_format09)
   if (19 != std::distance(begin,end))
      return false;
   else if (
             ('-' != *(begin +  2)) || ('-' != *(begin +  5)) ||
             (' ' != *(begin + 10)) || (':' != *(begin + 13)) ||
             (':' != *(begin + 16))
           )
      return false;
   else if (
             !strtk::fast::all_digits_check<2>(begin +  0) ||
             !strtk::fast::all_digits_check<2>(begin +  3) ||
             !strtk::fast::all_digits_check<4>(begin +  6) ||
             !strtk::fast::all_digits_check<2>(begin + 11) ||
             !strtk::fast::all_digits_check<2>(begin + 14) ||
             !strtk::fast::all_digits_check<2>(begin + 17)
           )
      return false;
   strtk::fast::numeric_convert<2>(begin +  0,t.dt.day   );
   strtk::fast::numeric_convert<2>(begin +  3,t.dt.month );
   strtk::fast::numeric_convert<4>(begin +  6,t.dt.year  );
   strtk::fast::numeric_convert<2>(begin + 11,t.dt.hour  );
   strtk::fast::numeric_convert<2>(begin + 14,t.dt.minute);
   strtk::fast::numeric_convert<2>(begin + 17,t.dt.second);
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::datetime_format10)
   if (19 != std::distance(begin,end))
      return false;
   else if (
             ('-' != *(begin +  4)) || ('-' != *(begin +  7)) ||
             ('T' != *(begin + 10)) || (':' != *(begin + 13)) ||
             (':' != *(begin + 16))
           )
      return false;
   else if (
             !strtk::fast::all_digits_check<4>(begin +  0) ||
             !strtk::fast::all_digits_check<2>(begin +  5) ||
             !strtk::fast::all_digits_check<2>(begin +  8) ||
             !strtk::fast::all_digits_check<2>(begin + 11) ||
             !strtk::fast::all_digits_check<2>(begin + 14) ||
             !strtk::fast::all_digits_check<2>(begin + 17)
           )
      return false;
   strtk::fast::numeric_convert<4>(begin +  0,t.dt.year  );
   strtk::fast::numeric_convert<2>(begin +  5,t.dt.month );
   strtk::fast::numeric_convert<2>(begin +  8,t.dt.day   );
   strtk::fast::numeric_convert<2>(begin + 11,t.dt.hour  );
   strtk::fast::numeric_convert<2>(begin + 14,t.dt.minute);
   strtk::fast::numeric_convert<2>(begin + 17,t.dt.second);
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::datetime_format11)
   if (23 != std::distance(begin,end))
      return false;
   else if (
             ('-' != *(begin +  4)) || ('-' != *(begin +  7)) ||
             ('T' != *(begin + 10)) || (':' != *(begin + 13)) ||
             (':' != *(begin + 16)) || ('.' != *(begin + 19))
           )
      return false;
   else if (
             !strtk::fast::all_digits_check<4>(begin +  0) ||
             !strtk::fast::all_digits_check<2>(begin +  5) ||
             !strtk::fast::all_digits_check<2>(begin +  8) ||
             !strtk::fast::all_digits_check<2>(begin + 11) ||
             !strtk::fast::all_digits_check<2>(begin + 14) ||
             !strtk::fast::all_digits_check<2>(begin + 17) ||
             !strtk::fast::all_digits_check<3>(begin + 20)
           )
      return false;
   strtk::fast::numeric_convert<4>(begin +  0,t.dt.year       );
   strtk::fast::numeric_convert<2>(begin +  5,t.dt.month      );
   strtk::fast::numeric_convert<2>(begin +  8,t.dt.day        );
   strtk::fast::numeric_convert<2>(begin + 11,t.dt.hour       );
   strtk::fast::numeric_convert<2>(begin + 14,t.dt.minute     );
   strtk::fast::numeric_convert<2>(begin + 17,t.dt.second     );
   strtk::fast::numeric_convert<3>(begin + 20,t.dt.millisecond);
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::datetime_format12)
   if (17 != std::distance(begin,end))
      return false;
   else if (!dt_utils::details::parse_YYYYMMDD(begin,begin + 8,t.dt))
      return false;
   else if (
             ('T' != *(begin +  8)) || (':' != *(begin + 11)) ||
             (':' != *(begin + 14))
           )
      return false;
   else if (
             !strtk::fast::all_digits_check<2>(begin +  9) ||
             !strtk::fast::all_digits_check<2>(begin + 12) ||
             !strtk::fast::all_digits_check<2>(begin + 15)
           )
      return false;
   strtk::fast::numeric_convert<2>(begin +  9,t.dt.hour  );
   strtk::fast::numeric_convert<2>(begin + 12,t.dt.minute);
   strtk::fast::numeric_convert<2>(begin + 15,t.dt.second);
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::datetime_format13)
   if (21 != std::distance(begin,end))
      return false;
   else if (!dt_utils::details::parse_YYYYMMDD(begin,begin + 8,t.dt))
      return false;
   else if (
             ('T' != *(begin +  8)) || (':' != *(begin + 11)) ||
             (':' != *(begin + 14))
           )
      return false;
   else if (
             !strtk::fast::all_digits_check<2>(begin +  9) ||
             !strtk::fast::all_digits_check<2>(begin + 12) ||
             !strtk::fast::all_digits_check<2>(begin + 15) ||
             !strtk::fast::all_digits_check<3>(begin + 18)
           )
      return false;
   strtk::fast::numeric_convert<2>(begin +  9,t.dt.hour       );
   strtk::fast::numeric_convert<2>(begin + 12,t.dt.minute     );
   strtk::fast::numeric_convert<2>(begin + 15,t.dt.second     );
   strtk::fast::numeric_convert<3>(begin + 18,t.dt.millisecond);
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::datetime_format14)
   if (23 != std::distance(begin,end))
      return false;
   else if (
             ('-' != *(begin +  2)) || ('-' != *(begin +  5)) ||
             ('T' != *(begin + 10)) || (':' != *(begin + 13)) ||
             (':' != *(begin + 16)) || ('.' != *(begin + 19))
           )
      return false;
   else if (
             !strtk::fast::all_digits_check<2>(begin +  0) ||
             !strtk::fast::all_digits_check<2>(begin +  3) ||
             !strtk::fast::all_digits_check<4>(begin +  6) ||
             !strtk::fast::all_digits_check<2>(begin + 11) ||
             !strtk::fast::all_digits_check<2>(begin + 14) ||
             !strtk::fast::all_digits_check<2>(begin + 17) ||
             !strtk::fast::all_digits_check<3>(begin + 20)
           )
      return false;
   strtk::fast::numeric_convert<2>(begin +  0,t.dt.day        );
   strtk::fast::numeric_convert<2>(begin +  3,t.dt.month      );
   strtk::fast::numeric_convert<4>(begin +  6,t.dt.year       );
   strtk::fast::numeric_convert<2>(begin + 11,t.dt.hour       );
   strtk::fast::numeric_convert<2>(begin + 14,t.dt.minute     );
   strtk::fast::numeric_convert<2>(begin + 17,t.dt.second     );
   strtk::fast::numeric_convert<3>(begin + 20,t.dt.millisecond);
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::datetime_format15)
   if (19 != std::distance(begin,end))
      return false;
   else if (
             ('-' != *(begin +  2)) || ('-' != *(begin +  5)) ||
             ('T' != *(begin + 10)) || (':' != *(begin + 13)) ||
             (':' != *(begin + 16))
           )
      return false;
   else if (
             !strtk::fast::all_digits_check<2>(begin +  0) ||
             !strtk::fast::all_digits_check<2>(begin +  3) ||
             !strtk::fast::all_digits_check<4>(begin +  6) ||
             !strtk::fast::all_digits_check<2>(begin + 11) ||
             !strtk::fast::all_digits_check<2>(begin + 14) ||
             !strtk::fast::all_digits_check<2>(begin + 17)
           )
      return false;
   strtk::fast::numeric_convert<2>(begin +  0,t.dt.day   );
   strtk::fast::numeric_convert<2>(begin +  3,t.dt.month );
   strtk::fast::numeric_convert<4>(begin +  6,t.dt.year  );
   strtk::fast::numeric_convert<2>(begin + 11,t.dt.hour  );
   strtk::fast::numeric_convert<2>(begin + 14,t.dt.minute);
   strtk::fast::numeric_convert<2>(begin + 17,t.dt.second);
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::datetime_format16)
   if (13 != std::distance(begin,end))
      return false;
   else if (!dt_utils::details::parse_YYYYMMDD(begin,begin + 8,t.dt))
      return false;
   else if ('T' != *(begin + 8))
      return false;
   else if (
             !strtk::fast::all_digits_check<2>(begin +  9) ||
             !strtk::fast::all_digits_check<2>(begin + 11)
           )
      return false;
   strtk::fast::numeric_convert<2>(begin +  9,t.dt.hour  );
   strtk::fast::numeric_convert<2>(begin + 11,t.dt.minute);
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::datetime_format17)
   if (15 != std::distance(begin,end))
      return false;
   else if (!dt_utils::details::parse_YYYYMMDD(begin,begin + 8,t.dt))
      return false;
   else if ('T' != *(begin + 8))
      return false;
   else if (
             !strtk::fast::all_digits_check<2>(begin +  9) ||
             !strtk::fast::all_digits_check<2>(begin + 11) ||
             !strtk::fast::all_digits_check<2>(begin + 13)
           )
      return false;
   strtk::fast::numeric_convert<2>(begin +  9,t.dt.hour  );
   strtk::fast::numeric_convert<2>(begin + 11,t.dt.minute);
   strtk::fast::numeric_convert<2>(begin + 13,t.dt.second);
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::datetime_format18)
   if (18 != std::distance(begin,end))
      return false;
   else if (!dt_utils::details::parse_YYYYMMDD(begin,begin + 8,t.dt))
      return false;
   else if ('T' != *(begin + 8))
      return false;
   else if (
             !strtk::fast::all_digits_check<2>(begin +  9) ||
             !strtk::fast::all_digits_check<2>(begin + 11) ||
             !strtk::fast::all_digits_check<2>(begin + 13) ||
             !strtk::fast::all_digits_check<3>(begin + 15)
           )
      return false;
   strtk::fast::numeric_convert<2>(begin +  9,t.dt.hour       );
   strtk::fast::numeric_convert<2>(begin + 11,t.dt.minute     );
   strtk::fast::numeric_convert<2>(begin + 13,t.dt.second     );
   strtk::fast::numeric_convert<3>(begin + 15,t.dt.millisecond);
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::datetime_format19)
   const std::size_t size = static_cast<std::size_t>(std::distance(begin,end));
   if ((20 != size) && (25 != size))
      return false;
   else if (
             ('-' != *(begin +  4)) || ('-' != *(begin +  7)) ||
             ('T' != *(begin + 10)) || (':' != *(begin + 13)) ||
             (':' != *(begin + 16))
           )
      return false;
   else if ((19 == size) && ('Z' != *(begin + 19)))
      return false;
   else if (
             (25 == size) &&
             (
              (('-' != *(begin + 19)) && ('+' != *(begin + 19)))
              ||
              (':' != *(begin + 22))
             )
           )
      return false;
   else if (
             !strtk::fast::all_digits_check<4>(begin +  0) ||
             !strtk::fast::all_digits_check<2>(begin +  5) ||
             !strtk::fast::all_digits_check<2>(begin +  8) ||
             !strtk::fast::all_digits_check<2>(begin + 11) ||
             !strtk::fast::all_digits_check<2>(begin + 14) ||
             !strtk::fast::all_digits_check<2>(begin + 17)
           )
      return false;
   strtk::fast::numeric_convert<4>(begin +  0,t.dt.year  );
   strtk::fast::numeric_convert<2>(begin +  5,t.dt.month );
   strtk::fast::numeric_convert<2>(begin +  8,t.dt.day   );
   strtk::fast::numeric_convert<2>(begin + 11,t.dt.hour  );
   strtk::fast::numeric_convert<2>(begin + 14,t.dt.minute);
   strtk::fast::numeric_convert<2>(begin + 17,t.dt.second);

   if (20 == size)
      t.dt.tzd = 0;
   else if (25 == size)
   {
      if (
           !strtk::fast::all_digits_check<2>(begin + 20) ||
           !strtk::fast::all_digits_check<2>(begin + 23)
         )
         return false;
      unsigned short tzd_hh;
      unsigned short tzd_mm;
      strtk::fast::numeric_convert<2>(begin + 20,tzd_hh);
      strtk::fast::numeric_convert<2>(begin + 23,tzd_mm);
      t.dt.tzd = ((tzd_hh * 60)  + tzd_mm) * (('-' == *(begin + 19)) ? -1 : 1);
   }
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::datetime_format20)
   const std::size_t size = static_cast<std::size_t>(std::distance(begin,end));
   if ((17 != size) && (22 != size))
      return false;
   else if (
             ('-' != *(begin +  4)) || ('-' != *(begin +  7)) ||
             ('T' != *(begin + 10)) || (':' != *(begin + 13))
           )
      return false;
   else if ((17 == size) && ('Z' != *(begin + 16)))
      return false;
   else if (
             (22 == size) &&
             (
              (('-' != *(begin + 16)) && ('+' != *(begin + 16)))
              ||
              (':' != *(begin + 19))
             )
           )
      return false;
   else if (
             !strtk::fast::all_digits_check<4>(begin +  0) ||
             !strtk::fast::all_digits_check<2>(begin +  5) ||
             !strtk::fast::all_digits_check<2>(begin +  8) ||
             !strtk::fast::all_digits_check<2>(begin + 11) ||
             !strtk::fast::all_digits_check<2>(begin + 14)
           )
      return false;
   strtk::fast::numeric_convert<4>(begin +  0,t.dt.year  );
   strtk::fast::numeric_convert<2>(begin +  5,t.dt.month );
   strtk::fast::numeric_convert<2>(begin +  8,t.dt.day   );
   strtk::fast::numeric_convert<2>(begin + 11,t.dt.hour  );
   strtk::fast::numeric_convert<2>(begin + 14,t.dt.minute);
   if (17 == size)
      t.dt.tzd = 0;
   else if (22 == size)
   {
      if (
           !strtk::fast::all_digits_check<2>(begin + 17) ||
           !strtk::fast::all_digits_check<2>(begin + 20)
         )
         return false;
      unsigned short tzd_hh;
      unsigned short tzd_mm;
      strtk::fast::numeric_convert<2>(begin + 17,tzd_hh);
      strtk::fast::numeric_convert<2>(begin + 20,tzd_mm);
      t.dt.tzd = ((tzd_hh * 60)  + tzd_mm) * (('-' == *(begin + 16)) ? -1 : 1);
   }
   return true;
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::datetime_format21)
   const std::size_t size = static_cast<std::size_t>(std::distance(begin,end));
   if (26 != size)
      return false;
   else if (
             ('/' != *(begin +  2)) || ('/' != *(begin +  6)) ||
             (':' != *(begin + 11)) || (':' != *(begin + 14)) ||
             (':' != *(begin + 17)) || (' ' != *(begin + 20))
           )
      return false;
   else if (
             ('-' != *(begin + 21)) && ('+' != *(begin + 21))
           )
      return false;
   else if (
             !strtk::fast::all_digits_check<2>(begin +  0) ||
             !strtk::fast::all_digits_check<4>(begin +  7) ||
             !strtk::fast::all_digits_check<2>(begin + 12) ||
             !strtk::fast::all_digits_check<2>(begin + 15) ||
             !strtk::fast::all_digits_check<2>(begin + 18) ||
             !strtk::fast::all_digits_check<4>(begin + 22)
           )
      return false;
   strtk::fast::numeric_convert<2>(begin +  0,t.dt.day   );
   strtk::fast::numeric_convert<4>(begin +  7,t.dt.year  );
   strtk::fast::numeric_convert<2>(begin + 12,t.dt.hour  );
   strtk::fast::numeric_convert<2>(begin + 15,t.dt.minute);
   strtk::fast::numeric_convert<2>(begin + 18,t.dt.second);
   unsigned short tzd_hh;
   unsigned short tzd_mm;
   strtk::fast::numeric_convert<2>(begin + 22,tzd_hh);
   strtk::fast::numeric_convert<2>(begin + 24,tzd_mm);
   t.dt.tzd = ((tzd_hh * 60)  + tzd_mm) * (('-' == *(begin + 21)) ? -1 : 1);
   return (0 != (t.dt.month = dt_utils::details::month3chr_to_index(begin + 3)));
strtk_string_to_type_end()

strtk_string_to_type_begin(dt_utils::datetime_format22)
   const std::size_t size = static_cast<std::size_t>(std::distance(begin,end));
   if (
        (27 != size) && (28 != size) &&
        (29 != size) && (31 != size)
      )
      return false;
   else if (
             (' ' != *(begin +  4)) || (' ' != *(begin +  7)) ||
             (' ' != *(begin + 11)) || (' ' != *(begin + 16)) ||
             (':' != *(begin + 19)) || (':' != *(begin + 22)) ||
             (' ' != *(begin + 25))
           )
      return false;
   else if (
             !strtk::fast::all_digits_check<2>(begin +  5) ||
             !strtk::fast::all_digits_check<4>(begin + 12) ||
             !strtk::fast::all_digits_check<2>(begin + 17) ||
             !strtk::fast::all_digits_check<2>(begin + 20) ||
             !strtk::fast::all_digits_check<2>(begin + 23) ||
             (0 == dt_utils::details::dow3chr_to_index(begin))
           )
      return false;
   strtk::fast::numeric_convert<2>(begin +  5,t.dt.day   );
   strtk::fast::numeric_convert<4>(begin + 12,t.dt.year  );
   strtk::fast::numeric_convert<2>(begin + 17,t.dt.hour  );
   strtk::fast::numeric_convert<2>(begin + 20,t.dt.minute);
   strtk::fast::numeric_convert<2>(begin + 23,t.dt.second);
   t.dt.tzd = 0;
   if (27 == size)
   {
      if (!dt_utils::details::miltzd1chr_to_offset(begin + 26,t.dt.tzd))
         return false;
   }
   else if (28 == size)
   {
      if (('U' != *(begin +  26)) || ('T' != *(begin +  27)))
         return false;
   }
   else if (29 == size)
   {
      if (!dt_utils::details::tzd3chr_to_offset(begin + 26,t.dt.tzd))
         return false;
   }
   else
   {
      unsigned short tzd_hh;
      unsigned short tzd_mm;
      strtk::fast::numeric_convert<2>(begin + 27,tzd_hh);
      strtk::fast::numeric_convert<2>(begin + 29,tzd_mm);
      t.dt.tzd = ((tzd_hh * 60)  + tzd_mm) * (('-' == *(begin + 26)) ? -1 : 1);
   }
   return (0 != (t.dt.month = dt_utils::details::month3chr_to_index(begin + 8)));
strtk_string_to_type_end()

#endif
