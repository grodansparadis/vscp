// C program calculating the sunrise and sunset for
// the current date and a fixed location(latitude,longitude)
// Note, twilight calculation gives insufficient accuracy of results
// Jarmo Lammi 1999 - 2001
// Last update July 21st, 2001

#include <stdio.h>
#include <math.h>
#include <stdlib.h> 
#include <time.h>

double pi = 3.14159;
double degs;
double rads;

double L, g, daylen;
double SunDia = 0.53; // Sunradius degrees

double AirRefr = 34.0 / 60.0; // athmospheric refraction degrees //

double test_longitude = 61.7441833;
double test_latitude = 15.1604167;

//   Get the days to J2000
//   h is UT in decimal hours
//   FNday only works between 1901 to 2099 - see Meeus chapter 7

double FNday(int y, int m, int d, float h)
{
    long int luku = -7 * (y + (m + 9) / 12) / 4 + 275 * m / 9 + d;
    // type casting necessary on PC DOS and TClite to avoid overflow
    luku += (long int) y * 367;
    return(double) luku - 730531.5 + h / 24.0;
};

//   the function below returns an angle in the range
//   0 to 2*pi

double FNrange(double x)
{
    double b = 0.5 * x / pi;
    double a = 2.0 * pi * (b - (long) (b));
    if (a < 0) a = 2.0 * pi + a;
    return a;
};

// Calculating the hourangle
//

double f0(double lat, double declin)
{
    double fo, dfo;
    // Correction: different sign at S HS
    dfo = rads * (0.5 * SunDia + AirRefr);
    if (lat < 0.0) dfo = -dfo;
    fo = tan(declin + dfo) * tan(lat * rads);
    if (fo > 0.99999) fo = 1.0; // to avoid overflow //
    fo = asin(fo) + pi / 2.0;
    return fo;
};

// Calculating the hourangle for twilight times
//

double f1(double lat, double declin)
{
    double fi, df1;
    // Correction: different sign at S HS
    df1 = rads * 6.0;
    if (lat < 0.0) df1 = -df1;
    fi = tan(declin + df1) * tan(lat * rads);
    if (fi > 0.99999) fi = 1.0; // to avoid overflow //
    fi = asin(fi) + pi / 2.0;
    return fi;
};


//   Find the ecliptic longitude of the Sun

double FNsun(double d)
{

    //   mean longitude of the Sun

    L = FNrange(280.461 * rads + .9856474 * rads * d);

    //   mean anomaly of the Sun

    g = FNrange(357.528 * rads + .9856003 * rads * d);

    //   Ecliptic longitude of the Sun

    return FNrange(L + 1.915 * rads * sin(g) + .02 * rads * sin(2 * g));
};


// Display decimal hours in hours and minutes

void showhrmn(double dhr)
{
    int hr, mn;
    hr = (int) dhr;
    mn = (dhr - (double) hr)*60;

    printf("%0d:%0d", hr, mn);
};

int main(void)
{

    double y, m, day, h, latit, longit;
    float inlat, inlon, intz;
    double tzone, d, lambda;
    double obliq, alpha, delta, LL, equation, ha, hb, twx;
    double twam, altmax, noont, settm, riset, twpm;
    time_t sekunnit;
    struct tm *p;

    degs = 180.0 / pi;
    rads = pi / 180.0;
    //  get the date and time from the user
    // read system date and extract the year

    /** First get time **/
    time(&sekunnit);

    /** Next get localtime **/

    p = localtime(&sekunnit);

    y = p->tm_year;
    // this is Y2K compliant method
    y += 1900;
    m = p->tm_mon + 1;

    day = p->tm_mday;

    h = 12;

    printf("year %4d month %2d\n", (int) y, (int) m);
    printf("Input latitude, longitude and timezone\n");
    //iscanf("%f", &inlat);
    //scanf("%f", &inlon);
    //scanf("%f", &intz);
    latit = test_longitude; //(double) inlat;
    longit = test_latitude; //(double) inlon;
    tzone = (double)2; // intz;

    printf("Latitude %f\n", latit );
    printf("Longitude %f\n", longit );
    
    // testing
    // m=6; day=10;


    d = FNday(y, m, day, h);
    printf("d %f\n", d );
    //   Use FNsun to find the ecliptic longitude of the
    //   Sun

    lambda = FNsun(d);

    //   Obliquity of the ecliptic

    obliq = 23.439 * rads - .0000004 * rads * d;

    //   Find the RA and DEC of the Sun

    alpha = atan2(cos(obliq) * sin(lambda), cos(lambda));
    delta = asin(sin(obliq) * sin(lambda));

    // Find the Equation of Time
    // in minutes
    // Correction suggested by David Smith
    LL = L - alpha;
    if (L < pi) LL += 2.0 * pi;
    equation = 1440.0 * (1.0 - LL / pi / 2.0);
    ha = f0(latit, delta);
    hb = f1(latit, delta);
    twx = hb - ha; // length of twilight in radians
    twx = 12.0 * twx / pi; // length of twilight in hours
    printf("ha= %.2f   hb= %.2f \n", ha, hb);
    // Conversion of angle to hours and minutes //
    daylen = degs * ha / 7.5;
    if (daylen < 0.0001) {
        daylen = 0.0;
    }
    // arctic winter     //

    riset = 12.0 - 12.0 * ha / pi + tzone - longit / 15.0 + equation / 60.0;
    settm = 12.0 + 12.0 * ha / pi + tzone - longit / 15.0 + equation / 60.0;
    noont = riset + 12.0 * ha / pi;
    altmax = 90.0 + delta * degs - latit;
    // Correction for S HS suggested by David Smith
    // to express altitude as degrees from the N horizon
    if (latit < delta * degs) altmax = 180.0 - altmax;

    twam = riset - twx; // morning twilight begin
    twpm = settm + twx; // evening twilight end

    if (riset > 24.0) riset -= 24.0;
    if (settm > 24.0) settm -= 24.0;

    puts("\n Sunrise and set");
    puts("===============");

    printf("  year  : %d \n", (int) y);
    printf("  month : %d \n", (int) m);
    printf("  day   : %d \n\n", (int) day);
    printf("Days since Y2K :  %d \n", (int) d);

    printf("Latitude :  %3.1f, longitude: %3.1f, timezone: %3.1f \n", (float) latit, (float) longit, (float) tzone);
    printf("Declination   :  %.2f \n", delta * degs);
    printf("Daylength     : ");
    showhrmn(daylen);
    puts(" hours \n");
    printf("Civil twilight: ");
    showhrmn(twam);
    puts("");
    printf("Sunrise       : ");
    showhrmn(riset);
    puts("");

    printf("Sun altitude ");
    // Amendment by D. Smith
    printf(" %.2f degr", altmax);
    printf(latit >= 0.0 ? " South" : " North");
    printf(" at noontime ");
    showhrmn(noont);
    puts("");
    printf("Sunset        : ");
    showhrmn(settm);
    puts("");
    printf("Civil twilight: ");
    showhrmn(twpm);
    puts("\n");

    return 0;
}
