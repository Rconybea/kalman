/* @file Time.hpp */

#pragma once

#include <iostream>
#include <iomanip>
#include <format>
#include <chrono>
#include <cstdint>
#include <time.h>

namespace xo {
  namespace time {

    using utc_nanos = std::chrono::time_point<std::chrono::system_clock,
					      std::chrono::nanoseconds>;

    using nanos = std::chrono::nanoseconds;
    using microseconds = std::chrono::microseconds;
    using seconds = std::chrono::seconds;
    using hours = std::chrono::hours;
    using days = std::chrono::days;

    struct Time {
      static utc_nanos epoch() {
	return std::chrono::system_clock::from_time_t(0);
      } /*epoch*/

      static utc_nanos ymd_hms(uint32_t ymd, uint32_t hms) {
        /* e.g. ymd=20220610 -> n_yr=2022, n_mon=06, n_dy=10 */

        uint32_t n_yr = ymd / 10000;
        uint32_t n_mon = (ymd % 10000) / 100;
        uint32_t n_dy = ymd % 100;

        uint32_t n_hr = hms / 10000;
        uint32_t n_min = (hms % 10000) / 100;
        uint32_t n_sec = hms % 100;

        struct tm t;

        t.tm_year = n_yr - 1900; /* 0 means 1900 */
        t.tm_mon = n_mon - 1;    /* 0 means january */
        t.tm_mday = n_dy;

        t.tm_hour = n_hr;	 /* 24 hour clock */
        t.tm_min = n_min;
        t.tm_sec = n_sec;

	/* time since epoch */
        time_t epoch_time = timegm(&t);

	return std::chrono::system_clock::from_time_t(epoch_time);
      } /*ymd_hms*/

      /* midnight UTC on date ymd.
       *   e.g. ymd_midnight(20220707) -> midnight UTC on 7jul22
       */
      static utc_nanos ymd_midnight(uint32_t ymd) {
	return ymd_hms(ymd, 0);
      } /*ymd_midnight*/

      static utc_nanos ymd_hms_usec(uint32_t ymd, uint32_t hms, uint32_t usec) {
	utc_nanos s = ymd_hms(ymd, hms);

	return s + microseconds(usec);
      } /*ymd_hms_usec*/

      static void print_ymd_hms_usec(utc_nanos t0, std::ostream & os) {
        using xo::time::microseconds;
        using xo::time::utc_nanos;

        /* use yyyymmdd.hhmmss.nnnnnnnnn */

        time_t t0_time_t = (std::chrono::system_clock::to_time_t
			    (std::chrono::time_point_cast<xo::time::microseconds>(t0)));

        /* convert to std::tm,
         * only provides 1-second precision
         */
        std::tm t0_tm;
        ::gmtime_r(&t0_time_t, &t0_tm);

        /* midnight on the same calendar day as t0_tm */
        std::tm midnight_tm = t0_tm;

        midnight_tm.tm_hour = 0;
        midnight_tm.tm_min = 0;
        midnight_tm.tm_sec = 0;

        /* convert back to epoch seconds */
        time_t midnight_time_t = ::mktime(&midnight_tm);

        utc_nanos t0_midnight =
            (std::chrono::time_point_cast<xo::time::microseconds>(
                std::chrono::system_clock::from_time_t(midnight_time_t)));

        uint32_t usec =
            (std::chrono::duration_cast<microseconds>(
                 std::chrono::hh_mm_ss(t0 - t0_midnight).subseconds()))
                .count();

        /* no std::format in clang11 afaict */
        char usec_buf[7];
        snprintf(usec_buf, sizeof(usec_buf), "%06d", usec);

        /* control string              | example
         * ----------------------------+--------------------------
         * %c - locale-specific string | Fri Jun 10 16:29:05 2022
         * %Y - year                   | 2022
         * %m - month                  | 06
         * %d - day of month           | 10
         * %H - hour                   | 16
         * %M - minute                 | 29
         * %S - second                 | 05
         * %Z - timezone               | UTC
         */
        os << std::put_time(&t0_tm, "%Y%m%d:%H%M%S.") << usec_buf;
      } /*print_ymd_hms_usec*/
    };  /*Time*/

  } /*namespace time*/
} /*namespace xo*/

namespace logutil {
inline std::ostream & operator<<(std::ostream & os,
				 xo::time::utc_nanos t0)
{
  xo::time::Time::print_ymd_hms_usec(t0, os);
  return os;
} /*operator<<*/
} /*namespace logutil*/

/* end Time.hpp */
