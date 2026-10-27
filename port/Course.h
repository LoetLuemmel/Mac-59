/* Course.h - the programming course's lessons and their checks against TI59Core, in the core's C++ subset
   so the host checker (port/coursecheck.cp) and the Mac's course window share one implementation.

   A lesson file is text, one keyword per line (German; MacRoman on the Mac, UTF-8 on the host):
     LEKTION <n> <title>        ZIEL <one sentence>
     TEXT                       explanation lines up to the next keyword
     TEIL <name>                Vormachen | Ueben | Probe - the five-part structure of a lesson
     SCHRITT <instruction>      a guided step;  AUFGABE <instruction>  a task without guidance
     VORBEREITUNG <keys>        keys run (by the checker and the course) before the step
       ERWARTE <condition>      all must hold for the step to count
       TIPP <text>              graded hints, first to last
       FEHLER <condition> : <text>   a known wrong state and what to say about it
       FALSCH <keys>            keys that produce that wrong state (checked by the host tool only)
       LOESUNG <keys>           the model solution (checked by the host tool only)
     PROBE                      lines "<keys> -> <number>" run in a copy of the calculator
   Conditions: modus LRN|HALT, anzeige "<text>", wert <number>, schritt <nnn>, zeiger <nnn>,
               register <nn> <number>, programm <nnn> <code> <code> ...
   Keys are names separated by blanks: 0-9 . +/- = + - x / ( ) A-E 2nd INV lnx CE CLR LRN x<>t x2 sqrt
   1/x SST STO RCL SUM y^x BST EE GTO SBR RST R/S, or matrix codes.
   (c) 2026 Pit Förster, with Claude Code via AppleBridge. */

#ifndef COURSE_H
#define COURSE_H

#include "TI59Core.h"

enum { kCourseMaxSteps = 32, kCourseMaxConds = 8, kCourseMaxErrors = 4, kCourseMaxHints = 3,
       kCourseMaxProbes = 8, kCourseText = 2000, kCourseLine = 160, kCourseKeys = 200 };

enum { kCondMode = 1, kCondDisplay, kCondValue, kCondStep, kCondPointer, kCondRegister, kCondProgram };

struct CourseCond {
    short kind;
    short a;                     /* step / register / pointer number; mode: 1 LRN, 0 HALT */
    double v;                    /* value */
    short codes[32], ncodes;     /* program codes */
    char text[24];               /* display text */
};

struct CourseStep {
    short task;                  /* 0 SCHRITT (guided), 1 AUFGABE */
    char part[16];               /* TEIL */
    char say[kCourseLine];
    short prep[kCourseKeys], nprep;
    CourseCond expect[kCourseMaxConds];
    short nexpect;
    char hint[kCourseMaxHints][kCourseLine];
    short nhint;
    CourseCond error[kCourseMaxErrors];
    char errorSay[kCourseMaxErrors][kCourseLine];
    short wrong[kCourseMaxErrors][kCourseKeys], nwrong[kCourseMaxErrors];
    short nerror;
    short solution[kCourseKeys], nsolution;
};

struct CourseProbe {
    short keys[kCourseKeys], nkeys;
    double want;
};

struct CourseLesson {
    short number;
    char title[kCourseLine];
    char goal[kCourseLine];
    char text[kCourseText];
    CourseStep step[kCourseMaxSteps];
    short nstep;
    CourseProbe probe[kCourseMaxProbes];
    short nprobe;
};

/* parse a lesson file; 0 and a message (line number) on an error */
short CourseParse(const char* buf, long n, CourseLesson* out, char* err);

/* key names -> matrix codes; returns the count, -1 on an unknown name */
short CourseKeys(const char* s, short* codes, short max);

/* reading the calculator */
short  CourseProgramCode(const TI59Core* m, short step);           /* 00..99 */
short  CoursePointer(const TI59Core* m);
double CourseRegister(const TI59Core* m, short reg);
void   CourseDisplayText(const TI59Display* d, char* out);          /* "-65.0000", "000 33", "1. -45" */
short  CourseIsLRN(const char* disp);
short  CourseDisplayValue(const char* disp, double* v);

/* one condition against the calculator and its current display text */
short CourseHolds(const CourseCond* c, const TI59Core* m, const char* disp);
/* the condition in words, for messages: "Schritt 002 enthält 85, erwartet 33" */
void  CourseDescribe(const CourseCond* c, const TI59Core* m, const char* disp, char* out);

#endif
