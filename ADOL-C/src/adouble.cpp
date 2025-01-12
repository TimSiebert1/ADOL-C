/*----------------------------------------------------------------------------
 ADOL-C -- Automatic Differentiation by Overloading in C++
 File:     adouble.cpp
 Revision: $Id$
 Contents: adouble.C contains that definitions of procedures used to
           define various badouble, adub, and adouble operations.
           These operations actually have two purposes.
           The first purpose is to actual compute the function, just as
           the same code written for double precision (single precision -
           complex - interval) arithmetic would.  The second purpose is
           to write a transcript of the computation for the reverse pass
           of automatic differentiation.

 Copyright (c) Andrea Walther, Andreas Griewank, Andreas Kowarz,
               Hristo Mitev, Sebastian Schlenkrich, Jean Utke, Olaf Vogel,
               Kshitij Kulshreshtha

 This file is part of ADOL-C. This software is provided as open source.
 Any use, reproduction, or distribution of the software constitutes
 recipient's acceptance of the terms of the accompanying license file.

----------------------------------------------------------------------------*/

#include "adouble.h"
#include "dvlparms.h"
#include "oplate.h"
#include "taping_p.h"
#include <adolc/adouble.h>

using namespace std;

/****************************************************************************/
/*                                                        HELPFUL FUNCTIONS */

/*--------------------------------------------------------------------------*/
void condassign(double &res, const double &cond, const double &arg1,
                const double &arg2) {
  res = cond > 0 ? arg1 : arg2;
}

/*--------------------------------------------------------------------------*/
void condassign(double &res, const double &cond, const double &arg) {
  res = cond > 0 ? arg : res;
}

/*--------------------------------------------------------------------------*/
void condeqassign(double &res, const double &cond, const double &arg1,
                  const double &arg2) {
  res = cond >= 0 ? arg1 : arg2;
}

/*--------------------------------------------------------------------------*/
void condeqassign(double &res, const double &cond, const double &arg) {
  res = cond >= 0 ? arg : res;
}
/*--------------------------------------------------------------------------*/
/* The remaining routines define the badouble, adub and adouble routines.   */
/*--------------------------------------------------------------------------*/

/****************************************************************************/
/*                     CONSTRUCTORS AND DESCTRUCTOR */

#if defined(ADOLC_ADOUBLE_LATEINIT)
void adouble::initInternal(void) {
  if (isInit)
    return;
  location = next_loc();
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

#if defined(ADOLC_ADOUBLE_STDCZERO)
  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[location]) {
#endif
      put_op(assign_d_zero);
      ADOLC_PUT_LOCINT(location); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[location]);
#if defined(ADOLC_TRACK_ACTIVITY)
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[location] = 0.;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[location] = false;
#endif
#endif
  isInit = true;
}
#else
void adouble::initInternal(void) {}
#endif

adouble::adouble() {
  tape_loc_{next_loc()};
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

#if defined(ADOLC_ADOUBLE_STDCZERO)
  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_]) {
#endif
      put_op(assign_d_zero);
      ADOLC_PUT_LOCINT(tape_loc_.loc_); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]);
#if defined(ADOLC_TRACK_ACTIVITY)
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_] = 0.;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_] = false;
#endif
#endif
}

adouble::adouble(double coval) {
  tape_loc_{next_loc()};
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_]) {
#endif
      if (coval == 0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(tape_loc_.loc_); // = res
      } else if (coval == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(tape_loc_.loc_); // = res
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(tape_loc_.loc_); // = res
        ADOLC_PUT_VAL(coval);             // = coval
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]);
#if defined(ADOLC_TRACK_ACTIVITY)
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_] = coval;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_] = false;
#endif
}

adouble::adouble(const tape_location &tape_loc) { tape_loc_{tape_loc}; }

adouble::adouble(const adouble &a) {
  tape_loc_{a.tape_loc_};
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif
      put_op(assign_a);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
      ADOLC_PUT_LOCINT(tape_loc_.loc_);   // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]);

#if defined(ADOLC_TRACK_ACTIVITY)
    } else {
      if (ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_]) {
        const double coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
        if (coval == 0) {
          put_op(assign_d_zero);
          ADOLC_PUT_LOCINT(tape_loc_.loc_); // = res
        } else if (coval == 1.0) {
          put_op(assign_d_one);
          ADOLC_PUT_LOCINT(tape_loc_.loc_); // = res
        } else {
          put_op(assign_d);
          ADOLC_PUT_LOCINT(tape_loc_.loc_); // = res
          ADOLC_PUT_VAL(coval);             // = coval
        }

        ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
        if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
          ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]);
      }
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_] =
      ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif
}

// Moving a's tape_location to the tape_location of this and set a's state to
// invalid (valid = 0). This will tell the destruction that "a" does not own its
// location. Thus, the location is not removed from the tape.
adouble::adouble(adouble &&a) noexcept {
  tape_loc_.loc_{a.tape_loc_.loc};
  a.valid = 0;
}

/*
 * The destructor is used to remove unused locations (tape_loc_.loc_) from the
 * tape. A location is only removed (free_loc), if the destructed adouble owns
 * the location. The adouble does not own its location if it is in an invalid
 * state (valid = 0). The state is only invalid, if the adouble was moved to a
 * new adouble. The location is reused for the new adouble in this case and must
 * remain on the tape.
 */
adouble::~adouble() {
#ifdef adolc_overwrite
  if (valid) {
    free_loc(tape_loc_.loc_);
  }
#endif
}

/****************************************************************************/
/*                                                              ASSIGNMENTS */

adouble &adouble::operator=(double coval) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_]) {
#endif
      if (coval == 0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(tape_loc_.loc_); // = res
      } else if (coval == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(tape_loc_.loc_); // = res
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(tape_loc_.loc_); // = res
        ADOLC_PUT_VAL(coval);             // = coval
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]);
#if defined(ADOLC_TRACK_ACTIVITY)
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_] = coval;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_] = false;
#endif
  return *this;
}

adouble &adouble::operator=(const adouble &a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  /* test this to avoid for x=x statements adjoint(x)=0 in reverse mode */
  if (tape_loc_.loc_ != a.tape_loc_.loc_) {
    if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
      if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif
        put_op(assign_a);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
        ADOLC_PUT_LOCINT(tape_loc_.loc_);   // = res

        ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
        if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
          ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]);
#if defined(ADOLC_TRACK_ACTIVITY)
      } else {
        if (ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_]) {
          double coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
          if (coval == 0) {
            put_op(assign_d_zero);
            ADOLC_PUT_LOCINT(tape_loc_.loc_); // = res
          } else if (coval == 1.0) {
            put_op(assign_d_one);
            ADOLC_PUT_LOCINT(tape_loc_.loc_); // = res
          } else {
            put_op(assign_d);
            ADOLC_PUT_LOCINT(tape_loc_.loc_); // = res
            ADOLC_PUT_VAL(coval);             // = coval
          }
          ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
          if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
            ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]);
        }
      }
#endif
    }
    ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_] =
        ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
#if defined(ADOLC_TRACK_ACTIVITY)
    ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_] =
        ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif
  }
  return *this;
}

// moves the tape_location of "a" to "this" and sets "a" to an invalid state.
adouble &adouble::operator=(adouble &&a) noexcept {
  if (this == &a) {
    return *this;
  }
  // remove location of this from tape to ensure it can be reused
  free_loc(tape_loc_.loc_);

  tape_loc_{a.tape_loc_.loc_};
  a.valid = 0;

  return *this;
}

/**************************************************************************
 *           MARK INDEPENDENT AND DEPENDENT
 */

// Assign a double value to an adouble and mark the adouble as independent on
// the tape
adouble &adouble::operator<<=(double input) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
    ++ADOLC_CURRENT_TAPE_INFOS.numInds;

    put_op(assign_ind);
    ADOLC_PUT_LOCINT(tape_loc_.loc_); // = res

    ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
    if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
      ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]);
  }
  ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_] = coval;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_] = true;
#endif
  return *this;
}

// Assign the coval of an adouble to a double reference and mark the adouble as
// dependent variable on the tape. At the end of the function, the double
// reference can be seen as output value of the function given by the trace
// of the adouble.
adouble &adouble::operator>>=(double &output) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
#if defined(ADOLC_TRACK_ACTIVITY)
  if (!ADOLC_GLOBAL_TAPE_VARS.actStore[loc()]) {
    fprintf(DIAG_OUT, "ADOL-C warning: marking an inactive variable (constant) "
                      "as dependent.\n");
    const double coval = ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_];
    if (coval == 0.0) {
      put_op(assign_d_zero);
      ADOLC_PUT_LOCINT(tape_loc_.loc_);
    } else if (coval == 1.0) {
      put_op(assign_d_one);
      ADOLC_PUT_LOCINT(tape_loc_.loc_);
    } else {
      put_op(assign_d);
      ADOLC_PUT_LOCINT(tape_loc_.loc_);
      ADOLC_PUT_VAL(coval);
    }

    ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
    if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
      ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]);
  }
#endif
  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
    ++ADOLC_CURRENT_TAPE_INFOS.numDeps;

    put_op(assign_dep);
    ADOLC_PUT_LOCINT(tape_loc_.loc_); // = res
  }

  coval = ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_];
  return *this;
}

void adouble::declareIndependent() {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
    ++ADOLC_CURRENT_TAPE_INFOS.numInds;

    put_op(assign_ind);
    ADOLC_PUT_LOCINT(tape_loc_.loc_); // = res

    ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
    if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
      ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]);
  }
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_] = true;
#endif
}

void adouble::declareDependent() {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
#if defined(ADOLC_TRACK_ACTIVITY)
  if (!ADOLC_GLOBAL_TAPE_VARS.actStore[loc()]) {
    fprintf(DIAG_OUT, "ADOL-C warning: marking an inactive variable (constant) "
                      "as dependent.\n");
    const double coval = ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_];
    if (coval == 0.0) {
      put_op(assign_d_zero);
      ADOLC_PUT_LOCINT(tape_loc_.loc_);
    } else if (coval == 1.0) {
      put_op(assign_d_one);
      ADOLC_PUT_LOCINT(tape_loc_.loc_);
    } else {
      put_op(assign_d);
      ADOLC_PUT_LOCINT(tape_loc_.loc_);
      ADOLC_PUT_VAL(coval);
    }

    ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
    if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
      ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]);
  }
#endif
  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
    ++ADOLC_CURRENT_TAPE_INFOS.numDeps;

    put_op(assign_dep);
    ADOLC_PUT_LOCINT(tape_loc_.loc_); // = res
  }
}

/****************************************************************************/
/*             Getter and Setter for the value stored at the tape location and
 * getter for location */

double adouble::getValue() const {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  return ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_];
}

double adouble::value() const { return getValue(); }

void adouble::setValue(const double coval) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_] = coval;
}
size_t adouble::getLoc() const { return tape_loc_.loc_; }

/****************************************************************************/
/*            conversions */

adouble::operator double() const {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  return ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_];
}

adouble::operator const double &() const {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  return ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_];
}

/****************************************************************************/
/*                                                           INPUT / OUTPUT */

std::ostream &operator<<(std::ostream &out, const adouble &a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  return out << a.getValue() << "(a)";
}

std::istream &operator>>(std::istream &in, const adouble &a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  double coval;
  in >> coval;
  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.getLoc()]) {
#endif
      if (coval == 0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(a.getLoc()); // = res
      } else if (coval == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(a.getLoc()); // = res
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(a.getLoc()); // = res
        ADOLC_PUT_VAL(coval);         // = coval
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.getLoc()]);
#if defined(ADOLC_TRACK_ACTIVITY)
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[a.getLoc()] = coval;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[a.getLoc()] = false;
#endif
  return in;
}

/****************************************************************************/
/* .                      ARITHMETIC ASSIGNMENT                             */

adouble &adouble::operator+=(const double coval) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_]) {
#endif
      put_op(eq_plus_d);
      ADOLC_PUT_LOCINT(tape_loc_.loc_); // = res
      ADOLC_PUT_VAL(coval);             // = coval

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]);
#if defined(ADOLC_TRACK_ACTIVITY)
    }
#endif
  }
  ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_] += coval;
  return *this;
}

adouble &adouble::operator+=(const adouble &a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] &&
        ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_]) {
#endif
      put_op(eq_plus_a);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
      ADOLC_PUT_LOCINT(tape_loc_.loc_);   // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]);
#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
      const double coval = ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_];
      if (coval) {
        put_op(plus_d_a);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
        ADOLC_PUT_LOCINT(tape_loc_.loc_);   // = res
        ADOLC_PUT_VAL(coval);
      } else {
        put_op(assign_a);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
        ADOLC_PUT_LOCINT(tape_loc_.loc_);   // = res
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]);

    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_]) {
      const double coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
      if (coval) {
        put_op(eq_plus_d);
        ADOLC_PUT_LOCINT(tape_loc_.loc_); // = res
        ADOLC_PUT_VAL(coval);             // = coval

        ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
        if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
          ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]);
      }
    }
#endif
  }
  ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_] +=
      ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_] =
      (ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_] ||
       ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]);
#endif
  return *this;
}

adouble &adouble::operator-=(const double coval) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_]) {
#endif
      put_op(eq_min_d);
      ADOLC_PUT_LOCINT(tape_loc_.loc_); // = res
      ADOLC_PUT_VAL(coval);             // = coval

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]);
#if defined(ADOLC_TRACK_ACTIVITY)
    }
#endif
  }
  ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_] -= coval;
  return *this;
}

adouble &adouble::operator-=(const adouble &a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] &&
        ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_]) {
#endif
      put_op(eq_min_a);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
      ADOLC_PUT_LOCINT(tape_loc_.loc_);   // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]);
#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
      const double coval = ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_];
      if (coval) {
        put_op(min_d_a);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
        ADOLC_PUT_LOCINT(tape_loc_.loc_);   // = res
        ADOLC_PUT_VAL(coval);
      } else {
        put_op(neg_sign_a);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
        ADOLC_PUT_LOCINT(tape_loc_.loc_);   // = res
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]);

    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_]) {
      const double coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
      if (coval) {
        put_op(eq_min_d);
        ADOLC_PUT_LOCINT(tape_loc_.loc_); // = res
        ADOLC_PUT_VAL(coval);             // = coval

        ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
        if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
          ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]);
      }
    }
#endif
  }
  ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_] -=
      ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_] =
      (ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_] ||
       ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]);
#endif
  return *this;
}

adouble &adouble::operator*=(const double coval) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_]) {
#endif
      put_op(eq_mult_d);
      ADOLC_PUT_LOCINT(tape_loc_.loc_); // = res
      ADOLC_PUT_VAL(coval);             // = coval

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]);
#if defined(ADOLC_TRACK_ACTIVITY)
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_] *= coval;
  return *this;
}

adouble &adouble::operator*=(const adouble &a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] &&
        ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_]) {
#endif
      put_op(eq_mult_a);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
      ADOLC_PUT_LOCINT(tape_loc_.loc_);   // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]);
#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
      const double coval = ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_];
      if (coval == -1.0) {
        put_op(neg_sign_a);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
        ADOLC_PUT_LOCINT(tape_loc_.loc_);   // = res
      } else if (coval == 1.0) {
        put_op(pos_sign_a);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
        ADOLC_PUT_LOCINT(tape_loc_.loc_);   // = res
      } else {
        put_op(mult_d_a);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
        ADOLC_PUT_LOCINT(tape_loc_.loc_);   // = res
        ADOLC_PUT_VAL(coval);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]);

    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_]) {
      const double coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
      put_op(eq_mult_d);
      ADOLC_PUT_LOCINT(tape_loc_.loc_); // = res
      ADOLC_PUT_VAL(coval);             // = coval

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_] *=
      ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_] =
      (ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_] ||
       ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]);
#endif
  return *this;
}

adouble &adouble::operator/=(const double coval) {
  *this *= 1.0 / coval;
  return *this;
}

adouble &adouble::operator/=(const adouble &a) {
  *this *= 1.0 / a;
  return *this;
}

/****************************************************************************/
/*                       INCREMENT / DECREMENT                              */

adouble adouble::operator++(int) {
  // create adouble to store old state in it.
  adouble ret_adouble(tape_loc{next_loc()});
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_]) {
#endif
      put_op(assign_a);
      ADOLC_PUT_LOCINT(tape_loc_.loc_);             // = arg
      ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
#if defined(ADOLC_TRACK_ACTIVITY)
    } else {
      if (ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_]) {
        const double coval = ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_];
        if (coval == 0) {
          put_op(assign_d_zero);
          ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res
        } else if (coval == 1.0) {
          put_op(assign_d_one);
          ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res
        } else {
          put_op(assign_d);
          ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res
          ADOLC_PUT_VAL(coval);                         // = coval
        }

        ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
        if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
          ADOLC_WRITE_SCAYLOR(
              ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
      }
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_] =
      ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_];
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_];
#endif

  // change input adouble to new state
  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_]) {
#endif
      put_op(incr_a);
      ADOLC_PUT_LOCINT(tape_loc_.loc_); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_)]);
#if defined(ADOLC_TRACK_ACTIVITY)
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]++;
  return ret_adouble;
}

adouble adouble::operator--(int) {
  // create adouble to store old state in it.
  adouble ret_adouble(tape_loc{next_loc()});
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_]) {
#endif
      put_op(assign_a);
      ADOLC_PUT_LOCINT(tape_loc_.loc_);             // = arg
      ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
#if defined(ADOLC_TRACK_ACTIVITY)
    } else {
      if (ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_]) {
        const double coval = ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_];
        if (coval == 0) {
          put_op(assign_d_zero);
          ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res
        } else if (coval == 1.0) {
          put_op(assign_d_one);
          ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res
        } else {
          put_op(assign_d);
          ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res
          ADOLC_PUT_VAL(coval);                         // = coval
        }

        ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
        if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
          ADOLC_WRITE_SCAYLOR(
              ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
      }
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_] =
      ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_];
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_];
#endif

  // write new state into input adouble
  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_]) {
#endif
      put_op(decr_a);
      ADOLC_PUT_LOCINT(tape_loc_.loc_); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]);
#if defined(ADOLC_TRACK_ACTIVITY)
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]--;
  return ret_adouble;
}

adouble &adouble::operator++() {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_]) {
#endif
      put_op(incr_a);
      ADOLC_PUT_LOCINT(tape_loc_.loc_); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]);
#if defined(ADOLC_TRACK_ACTIVITY)
    }
#endif
  }
  ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]++;
  return *this;
}

adouble &adouble::operator--() {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[tape_loc_.loc_]) {
#endif
      put_op(decr_a);
      ADOLC_PUT_LOCINT(tape_loc_.loc_); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]);
#if defined(ADOLC_TRACK_ACTIVITY)
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[tape_loc_.loc_]--;
  return *this;
}

/****************************************************************************/
/*                               COMPARISON                                 */

#ifdef ADOLC_ADVANCED_BRANCHING

adouble operator!=(const adouble &a, const adouble &b) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  const double a_coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
  const double b_coval = ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];
  adouble ret_adouble(tape_location{next_loc()});
  const double res = static_cast<double>(a_coval != b_coval);

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
    put_op(neq_a_a);
    ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // arg
    ADOLC_PUT_LOCINT(b.tape_loc_.loc_);           // arg1
    ADOLC_PUT_VAL(res);                           // check for branch switch
    ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // res

    ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
    if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
      ADOLC_WRITE_SCAYLOR(
          ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
  }

  ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_] = res;
  return ret_adouble;
}

adouble operator!=(adouble &&a, const adouble &b) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  const double a_coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
  const double b_coval = ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];
  const double res = static_cast<double>(a_coval != b_coval);

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
    put_op(neq_a_a);
    ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // arg
    ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // arg1
    ADOLC_PUT_VAL(res);                 // check for branch switch
    ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // res

    ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
    if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
      ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
  }

  ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] = res;
  return std::move(a);
}

adouble operator!=(const adouble &a, adouble &&b) { return std::move(b) != a; }

adouble operator==(const adouble &a, const adouble &b) {
  adouble ret_adouble(tape_location{next_loc()});
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  const double a_coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
  const double b_coval = ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];
  const double res = static_cast<double>(a_coval == b_coval);
  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
    put_op(eq_a_a);
    ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // arg
    ADOLC_PUT_LOCINT(b.tape_loc_.loc_);           // arg1
    ADOLC_PUT_VAL(res);                           // check for branch switch
    ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // res

    ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
    if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
      ADOLC_WRITE_SCAYLOR(
          ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
  }
  ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_] = res;
  return ret_adouble;
}

adouble operator==(adouble &&a, const adouble &b) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  const double a_coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
  const double b_coval = ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];
  const double res = static_cast<double>(a_coval == b_coval);

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
    put_op(eq_a_a);
    ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // arg
    ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // arg1
    ADOLC_PUT_VAL(res);                 // check for branch switch
    ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // res

    ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
    if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
      ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
  }

  ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] = res;
  return std::move(a);
}

adouble operator==(const adouble &a, adouble &&b) { return std::move(b) == a; }

adouble operator<=(const adouble &a, const adouble &b) {
  adouble ret_adouble(tape_location{next_loc()});
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  const double a_coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
  const double b_coval = ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];
  const double res = static_cast<double>(a_coval <= b_coval);
  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
    put_op(le_a_a);
    ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // arg
    ADOLC_PUT_LOCINT(b.tape_loc_.loc_);           // arg1
    ADOLC_PUT_VAL(res);                           // check for branch switch
    ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // res

    ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
    if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
      ADOLC_WRITE_SCAYLOR(
          ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
  }
  ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_] = res;
  return ret_adouble;
}

adouble operator<=(adouble &&a, const adouble &b) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  const double a_coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
  const double b_coval = ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];
  const double res = static_cast<double>(a_coval <= b_coval);

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
    put_op(le_a_a);
    ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // arg
    ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // arg1
    ADOLC_PUT_VAL(res);                 // check for branch switch
    ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // res

    ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
    if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
      ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
  }

  ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] = res;
  return std::move(a);
}

adouble operator<=(const adouble &a, adouble &&b) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  const double a_coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
  const double b_coval = ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];
  const double res = static_cast<double>(a_coval <= b_coval);

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
    put_op(le_a_a);
    ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // arg
    ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // arg1
    ADOLC_PUT_VAL(res);                 // check for branch switch
    ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // res

    ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
    if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
      ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_]);
  }

  ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_] = res;
  return std::move(b);
}
adouble operator>=(const adouble &a, const adouble &b) {
  adouble ret_adouble(tape_location{next_loc()});
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  const double a_coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
  const double b_coval = ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];
  const double res = static_cast<double>(a_coval >= b_coval);
  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
    put_op(ge_a_a);
    ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // arg
    ADOLC_PUT_LOCINT(b.tape_loc_.loc_);           // arg1
    ADOLC_PUT_VAL(res);                           // check for branch switch
    ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // res

    ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
    if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
      ADOLC_WRITE_SCAYLOR(
          ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
  }
  ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_] = res;
  return ret_adouble;
}

adouble operator>=(adouble &&a, const adouble &b) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  const double a_coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
  const double b_coval = ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];
  const double res = static_cast<double>(a_coval >= b_coval);

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
    put_op(ge_a_a);
    ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // arg
    ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // arg1
    ADOLC_PUT_VAL(res);                 // check for branch switch
    ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // res

    ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
    if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
      ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
  }

  ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] = res;
  return std::move(a);
}

adouble operator>=(const adouble &a, adouble &&b) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  const double a_coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
  const double b_coval = ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];
  const double res = static_cast<double>(a_coval >= b_coval);

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
    put_op(ge_a_a);
    ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // arg
    ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // arg1
    ADOLC_PUT_VAL(res);                 // check for branch switch
    ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // res

    ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
    if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
      ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_]);
  }

  ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_] = res;
  return std::move(b);
}

adouble operator<(const adouble &a, const adouble &b) {
  adouble ret_adouble(tape_location{next_loc()});
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  const double a_coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
  const double b_coval = ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];
  const double res = static_cast<double>(a_coval < b_coval);
  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
    put_op(lt_a_a);
    ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // arg
    ADOLC_PUT_LOCINT(b.tape_loc_.loc_);           // arg1
    ADOLC_PUT_VAL(res);                           // check for branch switch
    ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // res

    ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
    if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
      ADOLC_WRITE_SCAYLOR(
          ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
  }
  ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_] = res;
  return ret_adouble;
}

adouble operator<(adouble &&a, const adouble &b) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  const double a_coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
  const double b_coval = ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];
  const double res = static_cast<double>(a_coval < b_coval);

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
    put_op(lt_a_a);
    ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // arg
    ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // arg1
    ADOLC_PUT_VAL(res);                 // check for branch switch
    ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // res

    ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
    if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
      ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
  }

  ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] = res;
  return std::move(a);
}

adouble operator<(const adouble &a, adouble &&b) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  const double a_coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
  const double b_coval = ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];
  const double res = static_cast<double>(a_coval < b_coval);

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
    put_op(lt_a_a);
    ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // arg
    ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // arg1
    ADOLC_PUT_VAL(res);                 // check for branch switch
    ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // res

    ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
    if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
      ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_]);
  }

  ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_] = res;
  return std::move(b);
}

adouble operator>(const adouble &a, const adouble &b) {
  adouble ret_adouble(tape_location{next_loc()});
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  const double a_coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
  const double b_coval = ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];
  const double res = static_cast<double>(a_coval > b_coval);
  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
    put_op(gt_a_a);
    ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // arg
    ADOLC_PUT_LOCINT(b.tape_loc_.loc_);           // arg1
    ADOLC_PUT_VAL(res);                           // check for branch switch
    ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // res

    ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
    if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
      ADOLC_WRITE_SCAYLOR(
          ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
  }
  ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_] = res;
  return ret_adouble;
}

adouble operator>(adouble &&a, const adouble &b) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  const double a_coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
  const double b_coval = ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];
  const double res = static_cast<double>(a_coval > b_coval);

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
    put_op(gt_a_a);
    ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // arg
    ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // arg1
    ADOLC_PUT_VAL(res);                 // check for branch switch
    ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // res

    ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
    if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
      ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
  }

  ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] = res;
  return std::move(a);
}

adouble operator>(const adouble &a, adouble &&b) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  const double a_coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
  const double b_coval = ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];
  const double res = static_cast<double>(a_coval > b_coval);

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
    put_op(gt_a_a);
    ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // arg
    ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // arg1
    ADOLC_PUT_VAL(res);                 // check for branch switch
    ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // res

    ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
    if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
      ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_]);
  }

  ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_] = res;
  return std::move(b);
}

#else // ADOLC_ADVANCED_BRANCHING

bool operator!=(const adouble &a, const adouble &b) { return (a - b != 0); }

bool operator==(const badouble &u, const badouble &b) { return (a - b == 0); }

bool operator<=(const badouble &u, const badouble &b) { return (a - b <= 0); }

bool operator>=(const badouble &a, const badouble &b) { return (a - b >= 0); }

bool operator>(const badouble &a, const badouble &b) { return (a - b > 0); }

bool operator<(const badouble &a, const badouble &b) { return (a - b < 0); }

#endif // ADOLC_ADVANCED_BRANCHING

bool operator!=(const adouble &a, const double coval) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  if (coval)
    return (-coval + a != 0);
  else {
    if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
      if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif
        put_op(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] ? neq_zero
                                                              : eq_zero);
        ADOLC_PUT_LOCIN(Ta.tape_loc_.loc_);
#if defined(ADOLC_TRACK_ACTIVITY)
      }
#endif
    }
    return (ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] != 0);
  }
}

bool operator!=(const double coval, const adouble &a) {
  if (coval)
    return (-coval + a != 0);
  else
    return (a != 0);
}

bool operator==(const adouble &a, const double coval) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  if (coval)
    return (-coval + a == 0);
  else {
    if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
      if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif
        put_op(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] ? neq_zero
                                                              : eq_zero);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
#if defined(ADOLC_TRACK_ACTIVITY)
      }
#endif
    }
    return (ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] == 0);
  }
}

inline bool operator==(const double coval, const adouble &a) {
  if (coval)
    return (-coval + a == 0);
  else
    return (a == 0);
}

bool operator<=(const adouble &a, const double coval) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  if (coval)
    return (-coval + a <= 0);
  else {
    bool b = (ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] <= 0);
    if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
      if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif
        put_op(b ? le_zero : gt_zero);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
#if defined(ADOLC_TRACK_ACTIVITY)
      }
#endif
    }
    return b;
  }
}

inline bool operator<=(const double coval, const adouble &a) {
  if (coval)
    return (-coval + a >= 0);
  else
    return (a >= 0);
}

bool operator>=(const adouble &a, const double coval) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  if (coval)
    return (-coval + a >= 0);
  else {
    bool b = (ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] >= 0);
    if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
      if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif
        put_op(b ? ge_zero : lt_zero);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
#if defined(ADOLC_TRACK_ACTIVITY)
      }
#endif
    }
    return b;
  }
}

bool operator>=(const double coval, const adouble &a) {
  if (coval)
    return (-coval + a <= 0);
  else
    return (a <= 0);
}

bool operator<(const adouble &a, const double coval) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  if (coval)
    return (-coval + a < 0);
  else {
    bool b = (ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] < 0);
    if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
      if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif
        put_op(b ? lt_zero : ge_zero);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
#if defined(ADOLC_TRACK_ACTIVITY)
      }
#endif
    }
    return b;
  }
}

inline bool operator<(const double coval, const adouble &a) {
  if (coval)
    return (-coval + a > 0);
  else
    return (a > 0);
}

bool operator>(const adouble &a, const double coval) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  if (coval)
    return (-coval + a > 0);
  else {
    bool b = (ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] > 0);
    if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
      if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif
        put_op(b ? gt_zero : le_zero);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
#if defined(ADOLC_TRACK_ACTIVITY)
      }
#endif
    }
    return b;
  }
}

inline bool operator>(const double coval, const adouble &a) {
  if (coval)
    return (-coval + a < 0);
  else
    return (a < 0);
}

/****************************************************************************/
/*                           SIGN  OPERATORS                                 */

adouble operator+(const adouble &a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  adouble ret_adouble(tape_location{next_loc()});
  const double coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif
      put_op(pos_sign_a);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // = arg
      ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_]) {
      if (coval == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
      } else if (coval == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
        ADOLC_PUT_VAL(coval);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_] = coval;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif
  return ret_adouble;
}

adouble operator+(adouble &&a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  const double coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {

#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif

      put_op(pos_sign_a);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

#if defined(ADOLC_TRACK_ACTIVITY)

    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
      if (coval == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
      } else if (coval == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
        ADOLC_PUT_VAL(coval);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
    }
#endif
  }
  return std::move(a);
}

adouble operator-(const adouble &a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  adouble ret_adouble(tape_location{next_loc()});
  const double coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {

#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif

      put_op(neg_sign_a);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // = arg
      ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);

#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_]) {

      if (-coval == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
      } else if (-coval == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
        ADOLC_PUT_VAL(-coval);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_] = -coval;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif
  return ret_adouble;
}

adouble operator-(adouble &&a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  const double coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {

#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif

      put_op(neg_sign_a);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {

      if (-coval == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
      } else if (-coval == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
        ADOLC_PUT_VAL(-coval);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
    }
#endif
  }
  ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] = -coval;
  return std::move(a);
}

/****************************************************************************/
/*                            BINARY OPERATORS                              */

adouble operator+(const adouble &a, const adouble &b) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  adouble ret_adouble(tape_location{next_loc()});
  const double coval2 = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] +
                        ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {

#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] &&
        ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]) {
#endif

      put_op(plus_a_a);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // = arg1
      ADOLC_PUT_LOCINT(b.tape_loc_.loc_);           // = arg2
      ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);

#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {

      put_op(plus_d_a);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // = arg
      ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res
      ADOLC_PUT_VAL(ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_]);

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);

    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]) {

      const double coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];

      if (coval) {
        put_op(plus_d_a);
        ADOLC_PUT_LOCINT(b.tape_loc_.loc_);           // = arg
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res
        ADOLC_PUT_VAL(coval);
      } else {
        put_op(pos_sign_a);
        ADOLC_PUT_LOCINT(b.tape_loc_.loc_);           // = arg
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);

    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_]) {

      if (coval2 == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
      } else if (coval2 == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
        ADOLC_PUT_VAL(coval2);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_] = coval2;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_] =
      (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] ||
       ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]);
#endif
  return ret_adouble;
}

adouble operator+(adouble &&a, const adouble &b) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  const double coval2 = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] +
                        ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {

#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] &&
        ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]) {
#endif

      put_op(plus_a_a);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg1
      ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = arg2
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {

      put_op(plus_d_a);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res
      ADOLC_PUT_VAL(ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_]);

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]) {

      const double coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];

      if (coval) {
        put_op(plus_d_a);
        ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = arg
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res
        ADOLC_PUT_VAL(coval);
      } else {
        put_op(pos_sign_a);
        ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = arg
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {

      if (coval2 == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
      } else if (coval2 == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
        ADOLC_PUT_VAL(coval2);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] = coval2;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] =
      (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] ||
       ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]);
#endif
  return std::move(a);
}

adouble operator+(const adouble &a, adouble &&b) { return std::move(b) + a; }

adouble operator+(const double coval, const adouble &a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  adouble ret_adouble(tape_location{next_loc()});
  const double coval2 = coval + ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif

      if (coval) {
        put_op(plus_d_a);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // = arg
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res
        ADOLC_PUT_VAL(coval);                         // = coval
      } else {
        put_op(pos_sign_a);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // = arg
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);

#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_]) {

      if (coval2 == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
      } else if (coval2 == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
        ADOLC_PUT_VAL(coval2);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_] = coval2;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif

  return ret_adouble;
}

adouble operator+(const double coval, adouble &&a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  const double coval2 = coval + ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif

      if (coval) {
        put_op(plus_d_a);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
        ADOLC_PUT_LOCINT(a.tape_loc_.loc);  // = res
        ADOLC_PUT_VAL(coval);               // = coval
      } else {
        put_op(pos_sign_a);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc]);

#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc]) {

      if (coval2 == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc);
      } else if (coval2 == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT([a.tape_loc_.loc_]);
        ADOLC_PUT_VAL(coval2);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] = coval2;
  return std::move(a);
}

adouble operator+(const adouble &a, const double coval) { return coval + a; }
adouble operator+(adouble &&a, const double coval) {
  return coval + std::move(a);
}

adouble operator-(const adouble &a, const adouble &b) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  adouble ret_adouble(tape_location{next_loc()});
  const double coval2 = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] -
                        ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)

    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] &&
        ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]) {

#endif
      put_op(min_a_a);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // = arg1
      ADOLC_PUT_LOCINT(b.tape_loc_.loc_);           // = arg2
      ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);

#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {

      const double coval = -ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];
      put_op(plus_d_a);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // = arg
      ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res
      ADOLC_PUT_VAL(coval);

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);

    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]) {

      const double coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
      if (coval) {
        put_op(min_d_a);
        ADOLC_PUT_LOCINT(b.tape_loc_.loc_);           // = arg
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res
        ADOLC_PUT_VAL(coval);
      } else {
        put_op(neg_sign_a);
        ADOLC_PUT_LOCINT(b.tape_loc_.loc_);           // = arg
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);

    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_]) {

      if (coval2 == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
      } else if (coval2 == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
        ADOLC_PUT_VAL(coval2);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_] = coval2;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_] =
      (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] ||
       ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]);
#endif
  return ret_adouble;
}

adouble operator-(adouble &&a, const adouble &b) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  const double coval2 = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] -
                        ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)

    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] &&
        ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]) {

#endif
      put_op(min_a_a);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg1
      ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = arg2
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {

      const double coval = -ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];
      put_op(plus_d_a);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res
      ADOLC_PUT_VAL(coval);

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]) {

      const double coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
      if (coval) {
        put_op(min_d_a);
        ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = arg
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res
        ADOLC_PUT_VAL(coval);
      } else {
        put_op(neg_sign_a);
        ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = arg
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {

      if (coval2 == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
      } else if (coval2 == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
        ADOLC_PUT_VAL(coval2);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] = coval2;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] =
      (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] ||
       ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]);
#endif
  return std::move(a);
}

adouble operator-(const adouble &a, adouble &&b) { return -(std::move(b)) + a; }

adouble operator-(const double coval, const adouble &a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  adouble ret_adouble(tape_location{next_loc()});
  const double coval2 = coval - ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif

      if (coval) {
        put_op(min_d_a);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // = arg
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res
        ADOLC_PUT_VAL(coval);                         // = coval
      } else {
        put_op(neg_sign_a);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // = arg
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);

#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_]) {

      if (coval2 == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
      } else if (coval2 == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
        ADOLC_PUT_VAL(coval2);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_] = coval2;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif

  return ret_adouble;
}

adouble operator-(const double coval, adouble &&a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  const double coval2 = coval - ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif

      if (coval) {
        put_op(min_d_a);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res
        ADOLC_PUT_VAL(coval);               // = coval
      } else {
        put_op(neg_sign_a);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {

      if (coval2 == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
      } else if (coval2 == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
        ADOLC_PUT_VAL(coval2);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] = coval2;

  return std::move(a);
}

adouble operator-(const adouble &a, const double coval) { return (-coval) + a; }
adouble operator-(adouble &&a, const double coval) {
  return (-coval) + std::move(a);
}

adouble operator*(const adouble &a, const adouble &b) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  adouble ret_adouble(tape_location{next_loc()});
  const double coval2 = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] *
                        ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {

#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] &&
        ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]) {
#endif

      put_op(mult_a_a);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // = arg1
      ADOLC_PUT_LOCINT(b.tape_loc_.loc_);           // = arg2
      ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);

#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {

      const double coval = ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];
      put_op(mult_d_a);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // = arg
      ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res
      ADOLC_PUT_VAL(coval);

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);

    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]) {
      const double coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];

      if (coval == -1.0) {
        put_op(neg_sign_a);
        ADOLC_PUT_LOCINT(b.tape_loc_.loc_);           // = arg
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res
      } else if (coval == 1.0) {
        put_op(pos_sign_a);
        ADOLC_PUT_LOCINT(b.tape_loc_.loc_);           // = arg
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res
      } else {
        put_op(mult_d_a);
        ADOLC_PUT_LOCINT(b.tape_loc_.loc_);           // = arg
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res
        ADOLC_PUT_VAL(coval);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);

    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_]) {

      if (coval2 == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
      } else if (coval2 == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
        ADOLC_PUT_VAL(coval2);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_] = coval2;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_] =
      (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] ||
       ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]);
#endif
  return ret_adouble;
}

adouble operator*(adouble &&a, const adouble &b) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  const double coval2 = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] *
                        ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {

#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] &&
        ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]) {
#endif

      put_op(mult_a_a);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg1
      ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = arg2
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {

      const double coval = ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];
      put_op(mult_d_a);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res
      ADOLC_PUT_VAL(coval);

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]) {
      const double coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];

      if (coval == -1.0) {
        put_op(neg_sign_a);
        ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = arg
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res
      } else if (coval == 1.0) {
        put_op(pos_sign_a);
        ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = arg
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res
      } else {
        put_op(mult_d_a);
        ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = arg
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res
        ADOLC_PUT_VAL(coval);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {

      if (coval2 == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
      } else if (coval2 == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
        ADOLC_PUT_VAL(coval2);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] = coval2;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] =
      (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] ||
       ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]);
#endif
  return std::move(a);
}

adouble operator*(const adouble &a, adouble &&b) { return std::move(b) * a; }

adouble operator*(const double coval, const adouble &a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  adouble ret_adouble(tape_location{next_loc()});
  const double coval2 = coval * ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {

#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif

      if (coval == 1.0) {
        put_op(pos_sign_a);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // = arg
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res
      } else if (coval == -1.0) {
        put_op(neg_sign_a);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // = arg
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res
      } else {
        put_op(mult_d_a);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // = arg
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res
        ADOLC_PUT_VAL(coval);                         // = coval
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);

#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[locat]) {

      if (coval2 == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
      } else if (coval2 == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
        ADOLC_PUT_VAL(coval2);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_] = coval2;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif

  return ret_adouble;
}

adouble operator*(const double coval, adouble &&a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  const double coval2 = coval * ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {

#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif

      if (coval == 1.0) {
        put_op(pos_sign_a);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res
      } else if (coval == -1.0) {
        put_op(neg_sign_a);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res
      } else {
        put_op(mult_d_a);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res
        ADOLC_PUT_VAL(coval);               // = coval
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[locat]) {

      if (coval2 == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
      } else if (coval2 == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
        ADOLC_PUT_VAL(coval2);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] = coval2;
  return std::move(a);
}

adouble operator*(const adouble &a, const double coval) { return coval * a; }
adouble operator*(adouble &&a, const double coval) {
  return coval * std::move(a);
}

adouble operator/(const adouble &a, const adouble &b) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  adouble ret_adouble(tape_location{next_loc()});
  const double coval2 = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] /
                        ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)

    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] &&
        ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]) {
#endif

      put_op(div_a_a);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // = arg1
      ADOLC_PUT_LOCINT(b.tape_loc_.loc_);           // = arg2
      ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);

#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {

      const double coval = 1.0 / ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];

      if (coval == -1.0) {
        put_op(neg_sign_a);
        ADOLC_PUT_LOCINT(b.tape_loc_.loc_);           // = arg
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res
      } else if (coval == 1.0) {
        put_op(pos_sign_a);
        ADOLC_PUT_LOCINT(b.tape_loc_.loc_);           // = arg
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res
      } else {
        put_op(mult_d_a);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // = arg
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res
        ADOLC_PUT_VAL(coval);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);

    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]) {

      const double coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];

      put_op(div_d_a);
      ADOLC_PUT_LOCINT(b.tape_loc_.loc_);           // = arg
      ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res
      ADOLC_PUT_VAL(coval);

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);

    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_]) {

      if (coval2 == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
      } else if (coval2 == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
        ADOLC_PUT_VAL(coval2);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_] = coval2;

#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_] =
      (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] ||
       ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]);
#endif
  return ret_adouble;
}

adouble operator/(adouble &&a, const adouble &b) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  const double coval2 = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] /
                        ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)

    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] &&
        ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]) {
#endif

      put_op(div_a_a);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg1
      ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = arg2
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {

      const double coval = 1.0 / ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];

      if (coval == -1.0) {
        put_op(neg_sign_a);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res
      } else if (coval == 1.0) {
        put_op(pos_sign_a);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res
      } else {
        put_op(mult_d_a);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res
        ADOLC_PUT_VAL(coval);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]) {

      const double coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];

      put_op(div_d_a);
      ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = arg
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res
      ADOLC_PUT_VAL(coval);

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] = coval2;

#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] =
      (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] ||
       ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]);
#endif
  return std::move(a);
}

adouble operator/(const adouble &a, adouble &&b) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  const double coval2 = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] /
                        ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
#if defined(ADOLC_TRACK_ACTIVITY)

    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] &&
        ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]) {
#endif

      put_op(div_a_a);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg1
      ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = arg2
      ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_]);

#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {

      const double coval = 1.0 / ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];

      if (coval == -1.0) {
        put_op(neg_sign_a);
        ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = arg
        ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = res
      } else if (coval == 1.0) {
        put_op(pos_sign_a);
        ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = arg
        ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = res
      } else {
        put_op(mult_d_a);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
        ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = res
        ADOLC_PUT_VAL(coval);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_]);

    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]) {

      const double coval = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];

      put_op(div_d_a);
      ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = arg
      ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = res
      ADOLC_PUT_VAL(coval);

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_] = coval2;

#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_] =
      (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] ||
       ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]);
#endif
  return std::move(b);
}

adouble operator/(const double coval, const adouble &a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  adouble ret_adouble(tape_location{next_loc()});

  cosnt double coval2 = coval / ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {

#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif

      put_op(div_d_a);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // = arg
      ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res
      ADOLC_PUT_VAL(coval);                         // = coval

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);

#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_]) {

      if (coval2 == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
      } else if (coval2 == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
        ADOLC_PUT_VAL(coval2);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_] = coval2;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif
  return ret_adouble;
}

adouble operator/(const double coval, adouble &&a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  cosnt double coval2 = coval / ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {

#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif

      put_op(div_d_a);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res
      ADOLC_PUT_VAL(coval);               // = coval

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

#if defined(ADOLC_TRACK_ACTIVITY)

    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {

      if (coval2 == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
      } else if (coval2 == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
        ADOLC_PUT_VAL(coval2);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] = coval2;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif
  return std::move(a);
}

adouble operator/(const adouble &a, const double coval) {
  return a * 1.0 / coval;
}
adouble operator/(adouble &&a, const double coval) {
  return std::move(a) * 1.0 / coval;
}
/****************************************************************************/
/*                          UNARY OPERATIONS                                */

adouble exp(const adouble &a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  adouble ret_adouble(tape_location{next_loc()});
  const double coval =
      ADOLC_MATH_NSP::exp(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {

#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif

      put_op(exp_op);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // = arg
      ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);

#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_]) {

      if (coval == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
      } else if (coval == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
        ADOLC_PUT_VAL(coval);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_] = coval;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif

  ADOLC_OPENMP_RESTORE_THREAD_NUMBER;
  return ret_adouble;
}

adouble exp(adouble &&a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  const double coval =
      ADOLC_MATH_NSP::exp(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {

#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif

      put_op(exp_op);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {

      if (coval == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
      } else if (coval == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
        ADOLC_PUT_VAL(coval);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] = coval;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif

  ADOLC_OPENMP_RESTORE_THREAD_NUMBER;
  return std::move(a);
}

adouble log(const adouble &a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  adouble ret_adouble(tape_locaton{next_loc()});
  const double coval =
      ADOLC_MATH_NSP::log(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {

#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif

      put_op(log_op);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // = arg
      ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);

#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_]) {

      if (coval == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
      } else if (coval == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
        ADOLC_PUT_VAL(coval);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_] = coval;

#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif

  ADOLC_OPENMP_RESTORE_THREAD_NUMBER;
  return ret_adouble;
}

adouble log(adouble &&a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  const double coval =
      ADOLC_MATH_NSP::log(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {

#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif

      put_op(log_op);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {

      if (coval == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
      } else if (coval == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
        ADOLC_PUT_VAL(coval);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] = coval;

#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif

  ADOLC_OPENMP_RESTORE_THREAD_NUMBER;
  return std::move(a);
}

adouble sqrt(const adouble &a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  adouble ret_adouble(tape_location{next_loc()});
  const double coval =
      ADOLC_MATH_NSP::sqrt(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {

#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif

      put_op(sqrt_op);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // = arg
      ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);

#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_]) {

      if (coval == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
      } else if (coval == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
        ADOLC_PUT_VAL(coval);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_] = coval;

#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif

  ADOLC_OPENMP_RESTORE_THREAD_NUMBER;
  return ret_adouble;
}

adouble sqrt(adouble &&a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  const double coval =
      ADOLC_MATH_NSP::sqrt(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {

#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif

      put_op(sqrt_op);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {

      if (coval == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
      } else if (coval == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
        ADOLC_PUT_VAL(coval);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] = coval;

#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif

  ADOLC_OPENMP_RESTORE_THREAD_NUMBER;
  return std::move(a);
}

adouble cbrt(const adouble &a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  adouble ret_adouble(tape_location{next_loc()});
  const double coval =
      ADOLC_MATH_NSP::cbrt(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {

#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif

      put_op(cbrt_op);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // = arg
      ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);

#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_]) {

      if (coval == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
      } else if (coval == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
        ADOLC_PUT_VAL(coval);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_] = coval;

#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif

  ADOLC_OPENMP_RESTORE_THREAD_NUMBER;
  return ret_adouble;
}

adouble cbrt(adouble &&a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  const double coval =
      ADOLC_MATH_NSP::cbrt(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {

#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif

      put_op(cbrt_op);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {

      if (coval == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
      } else if (coval == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
        ADOLC_PUT_VAL(coval);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] = coval;

#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif

  ADOLC_OPENMP_RESTORE_THREAD_NUMBER;
  return std::move(a);
}

adouble sin(const adouble &a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  adouble ret_adouble(tape_location{next_loc()});

  const double coval1 =
      ADOLC_MATH_NSP::sin(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
  const double coval2 =
      ADOLC_MATH_NSP::cos(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

  adouble b;

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {

#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif

      put_op(sin_op);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // = arg1
      ADOLC_PUT_LOCINT(b.tape_loc_.loc_);           // = arg2
      ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res

      ADOLC_CURRENT_TAPE_INFOS.numTays_Tape += 2;

      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors) {
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_]);
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
      }

#if defined(ADOLC_TRACK_ACTIVITY)
    } else {
      if (ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_]) {

        if (coval1 == 0.0) {
          put_op(assign_d_zero);
          ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
        } else if (coval1 == 1.0) {
          put_op(assign_d_one);
          ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
        } else {
          put_op(assign_d);
          ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
          ADOLC_PUT_VAL(coval1);
        }

        ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;

        if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
          ADOLC_WRITE_SCAYLOR(
              ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
      }
      if (ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]) {

        if (coval2 == 0.0) {
          put_op(assign_d_zero);
          ADOLC_PUT_LOCINT(b.tape_loc_.loc_);
        } else if (coval1 == 1.0) {
          put_op(assign_d_one);
          ADOLC_PUT_LOCINT(b.tape_loc_.loc_);
        } else {
          put_op(assign_d);
          ADOLC_PUT_LOCINT(b.tape_loc_.loc_);
          ADOLC_PUT_VAL(coval2);
        }

        ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;

        if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
          ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_]);
      }
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_] = coval1;
  ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_] = coval2;

#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_] =
          ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif

  ADOLC_OPENMP_RESTORE_THREAD_NUMBER;
  return ret_adouble;
}

adouble sin(adouble &&a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  adouble a(tape_location{next_loc()});

  const double coval1 =
      ADOLC_MATH_NSP::sin(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
  const double coval2 =
      ADOLC_MATH_NSP::cos(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

  adouble b;

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {

#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif

      put_op(sin_op);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg1
      ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = arg2
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res

      ADOLC_CURRENT_TAPE_INFOS.numTays_Tape += 2;

      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors) {
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_]);
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
      }

#if defined(ADOLC_TRACK_ACTIVITY)
    } else {
      if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {

        if (coval1 == 0.0) {
          put_op(assign_d_zero);
          ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
        } else if (coval1 == 1.0) {
          put_op(assign_d_one);
          ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
        } else {
          put_op(assign_d);
          ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
          ADOLC_PUT_VAL(coval1);
        }

        ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;

        if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
          ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
      }
      if (ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]) {

        if (coval2 == 0.0) {
          put_op(assign_d_zero);
          ADOLC_PUT_LOCINT(b.tape_loc_.loc_);
        } else if (coval1 == 1.0) {
          put_op(assign_d_one);
          ADOLC_PUT_LOCINT(b.tape_loc_.loc_);
        } else {
          put_op(assign_d);
          ADOLC_PUT_LOCINT(b.tape_loc_.loc_);
          ADOLC_PUT_VAL(coval2);
        }

        ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;

        if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
          ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_]);
      }
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] = coval1;
  ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_] = coval2;

#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_] =
          ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif

  ADOLC_OPENMP_RESTORE_THREAD_NUMBER;
  return std::move(a);
}

adouble cos(const adouble &a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  adouble ret_adouble(tape_location{next_loc()});

  const double coval1 =
      ADOLC_MATH_NSP::cos(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
  const double coval2 =
      ADOLC_MATH_NSP::sin(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

  adouble b;

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {

#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif

      put_op(cos_op);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_);           // = arg1
      ADOLC_PUT_LOCINT(b.tape_loc_.loc_);           // = arg2
      ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_); // = res

      ADOLC_CURRENT_TAPE_INFOS.numTays_Tape += 2;

      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors) {
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_]);
        ADOLC_WRITE_SCAYLOR(
            ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
      }

#if defined(ADOLC_TRACK_ACTIVITY)

    } else {
      if (ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_]) {

        if (coval1 == 0.0) {
          put_op(assign_d_zero);
          ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
        } else if (coval1 == 1.0) {
          put_op(assign_d_one);
          ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
        } else {
          put_op(assign_d);
          ADOLC_PUT_LOCINT(ret_adouble.tape_loc_.loc_);
          ADOLC_PUT_VAL(coval1);
        }

        ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;

        if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
          ADOLC_WRITE_SCAYLOR(
              ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_]);
      }
      if (ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]) {

        if (coval2 == 0.0) {
          put_op(assign_d_zero);
          ADOLC_PUT_LOCINT(b.tape_loc_.loc_);
        } else if (coval1 == 1.0) {
          put_op(assign_d_one);
          ADOLC_PUT_LOCINT(b.tape_loc_.loc_);
        } else {
          put_op(assign_d);
          ADOLC_PUT_LOCINT(b.tape_loc_.loc_);
          ADOLC_PUT_VAL(coval2);
        }

        ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;

        if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
          ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_]);
      }
    }

#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[ret_adouble.tape_loc_.loc_] = coval1;
  ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_] = coval2;

#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[ret_adouble.tape_loc_.loc_] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_] =
          ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif

  ADOLC_OPENMP_RESTORE_THREAD_NUMBER;
  return ret_adouble;
}

adouble cos(adouble &&a) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;

  const double coval1 =
      ADOLC_MATH_NSP::cos(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
  const double coval2 =
      ADOLC_MATH_NSP::sin(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

  adouble b;

  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {

#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif

      put_op(cos_op);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg1
      ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = arg2
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = res

      ADOLC_CURRENT_TAPE_INFOS.numTays_Tape += 2;

      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors) {
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_]);
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
      }

#if defined(ADOLC_TRACK_ACTIVITY)

    } else {
      if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {

        if (coval1 == 0.0) {
          put_op(assign_d_zero);
          ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
        } else if (coval1 == 1.0) {
          put_op(assign_d_one);
          ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
        } else {
          put_op(assign_d);
          ADOLC_PUT_LOCINT(a.tape_loc_.loc_);
          ADOLC_PUT_VAL(coval1);
        }

        ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;

        if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
          ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
      }
      if (ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]) {

        if (coval2 == 0.0) {
          put_op(assign_d_zero);
          ADOLC_PUT_LOCINT(b.tape_loc_.loc_);
        } else if (coval1 == 1.0) {
          put_op(assign_d_one);
          ADOLC_PUT_LOCINT(b.tape_loc_.loc_);
        } else {
          put_op(assign_d);
          ADOLC_PUT_LOCINT(b.tape_loc_.loc_);
          ADOLC_PUT_VAL(coval2);
        }

        ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;

        if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
          ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_]);
      }
    }

#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_] = coval1;
  ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_] = coval2;

#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_] =
          ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif

  ADOLC_OPENMP_RESTORE_THREAD_NUMBER;
  return std::move(a);
}

/*--------------------------------------------------------------------------*/
/* Compute tan of adouble */
adub tan(const badouble &x) { return sin(x) / cos(x); }

/*--------------------------------------------------------------------------*/
/* Asin value -- really a quadrature */
adub asin(const badouble &x) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  locint locat = next_loc();
  double coval =
      ADOLC_MATH_NSP::asin(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

  adouble y = 1.0 / sqrt(1.0 - x * x);

  if (ADOLC_CURRENT_TAPE_INFOS
          .traceFlag) { // old:
                        // write_quad(asin_op,locat,a.tape_loc_.loc_,b.tape_loc_.loc_);
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS
            .actStore[a.tape_loc_.loc_]) { // y will have same activity as x and
                                           // can be
                                           // considered as second input here
#endif
      put_op(asin_op);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg1
      ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = arg2
      ADOLC_PUT_LOCINT(locat);            // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[locat]);
#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[locat]) {
      if (coval == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(locat);
      } else if (coval == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(locat);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(locat);
        ADOLC_PUT_VAL(coval);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[locat]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[locat] = coval;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[locat] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif
  ADOLC_OPENMP_RESTORE_THREAD_NUMBER;
  return locat;
}

/*--------------------------------------------------------------------------*/
/* Acos value -- really a quadrature */
adub acos(const badouble &x) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  locint locat = next_loc();
  double coval =
      ADOLC_MATH_NSP::acos(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

  adouble y = -1.0 / sqrt(1.0 - x * x);

  if (ADOLC_CURRENT_TAPE_INFOS
          .traceFlag) { // old:
                        // write_quad(acos_op,locat,a.tape_loc_.loc_,b.tape_loc_.loc_);
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS
            .actStore[a.tape_loc_.loc_]) { // y will have same activity as x and
                                           // can be
                                           // considered as second input here
#endif
      put_op(acos_op);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg1
      ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = arg2
      ADOLC_PUT_LOCINT(locat);            // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[locat]);
#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[locat]) {
      if (coval == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(locat);
      } else if (coval == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(locat);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(locat);
        ADOLC_PUT_VAL(coval);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[locat]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[locat] = coval;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[locat] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif
  ADOLC_OPENMP_RESTORE_THREAD_NUMBER;
  return locat;
}

/*--------------------------------------------------------------------------*/
/* Atan value -- really a quadrature */
adub atan(const badouble &x) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  locint locat = next_loc();
  double coval =
      ADOLC_MATH_NSP::atan(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

  adouble y = 1.0 / (1.0 + x * x);

  if (ADOLC_CURRENT_TAPE_INFOS
          .traceFlag) { // old:
                        // write_quad(atan_op,locat,a.tape_loc_.loc_,b.tape_loc_.loc_);
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS
            .actStore[a.tape_loc_.loc_]) { // y will have same activity as x and
                                           // can be
                                           // considered as second input here
#endif
      put_op(atan_op);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg1
      ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = arg2
      ADOLC_PUT_LOCINT(locat);            // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[locat]);
#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[locat]) {
      if (coval == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(locat);
      } else if (coval == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(locat);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(locat);
        ADOLC_PUT_VAL(coval);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[locat]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[locat] = coval;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[locat] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif
  ADOLC_OPENMP_RESTORE_THREAD_NUMBER;
  return locat;
}

/*--------------------------------------------------------------------------*/
adouble atan2(const badouble &y, const badouble &x) {
  adouble a1, a2, ret, sy;
  const double pihalf = ADOLC_MATH_NSP::asin(1.0);
  /* y+0.0 is a hack since condassign is currently not defined for
     badoubles */
  condassign(sy, y, (adouble)1.0, (adouble)-1.0);
  condassign(a1, x, atan(y / x), atan(y / x) + sy * 2 * pihalf);
  condassign(a2, fabs(y), sy * pihalf - atan(x / y), (adouble)0.0);
  condassign(ret, fabs(x) - fabs(y), a1, a2);
  return ret;
}

/*--------------------------------------------------------------------------*/
/* power value -- adouble ^ floating point */
adub pow(const badouble &x, double coval) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  locint locat = next_loc();
  double coval2 = ADOLC_MATH_NSP::pow(
      ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_], coval);

  if (ADOLC_CURRENT_TAPE_INFOS
          .traceFlag) { // old:
                        // write_args_d_a(pow_op,locat,cocval,a.tape_loc_.loc_);
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif
      put_op(pow_op);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
      ADOLC_PUT_LOCINT(locat);            // = res
      ADOLC_PUT_VAL(coval);               // = coval

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[locat]);
#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[locat]) {
      if (coval2 == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(locat);
      } else if (coval2 == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(locat);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(locat);
        ADOLC_PUT_VAL(coval2);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[locat]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[locat] = coval2;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[locat] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif
  ADOLC_OPENMP_RESTORE_THREAD_NUMBER;
  return locat;
}

/*--------------------------------------------------------------------------*/
/* power value --- floating point ^ adouble */
adouble pow(double coval, const badouble &y) {
  adouble ret;

  if (coval <= 0) {
    fprintf(DIAG_OUT, "\nADOL-C message:  exponent at zero/negative constant "
                      "basis deactivated\n");
  }

  condassign(ret, (adouble)coval, exp(y * ADOLC_MATH_NSP::log(coval)),
             (adouble)ADOLC_MATH_NSP::pow(coval, y.getValue()));

  return ret;
}

/*--------------------------------------------------------------------------*/
/* power value --- adouble ^ adouble */
adouble pow(const badouble &x, const badouble &y) {
  adouble a1, a2, ret;
  double vx = x.getValue();
  double vy = y.getValue();

  if (!(vx > 0)) {
    if (vx < 0 || vy >= 0)
      fprintf(
          DIAG_OUT,
          "\nADOL-C message: exponent of zero/negative basis deactivated\n");
    else
      fprintf(
          DIAG_OUT,
          "\nADOL-C message: negative exponent and zero basis deactivated\n");
  }
  condassign(a1, -y, (adouble)ADOLC_MATH_NSP::pow(vx, vy), pow(x, vy));
  condassign(a2, fabs(x), pow(x, vy), a1);
  condassign(ret, x, exp(y * log(x)), a2);

  return ret;
}

/*--------------------------------------------------------------------------*/
/* log base 10 of an adouble */
adub log10(const badouble &x) { return log(x) / ADOLC_MATH_NSP::log(10.0); }

/*--------------------------------------------------------------------------*/
/* Hyperbolic Sine of an adouble */
/* 981119 olvo changed as J.M. Aparicio suggested */
adub sinh(const badouble &x) {
  if (x.getValue() < 0.0) {
    adouble temp = exp(x);
    return 0.5 * (temp - 1.0 / temp);
  } else {
    adouble temp = exp(-x);
    return 0.5 * (1.0 / temp - temp);
  }
}

/*--------------------------------------------------------------------------*/
/* Hyperbolic Cosine of an adouble */
/* 981119 olvo changed as J.M. Aparicio suggested */
adub cosh(const badouble &x) {
  adouble temp = (x.getValue() < 0.0) ? exp(x) : exp(-x);
  return 0.5 * (temp + 1.0 / temp);
}

/*--------------------------------------------------------------------------*/
/*
  Hyperbolic Tangent of an adouble value.
*/
/* 981119 olvo changed as J.M. Aparicio suggested */
adub tanh(const badouble &x) {
  if (x.getValue() < 0.0) {
    adouble temp = exp(2.0 * x);
    return (temp - 1.0) / (temp + 1.0);
  } else {
    adouble temp = exp((-2.0) * x);
    return (1.0 - temp) / (temp + 1.0);
  }
}

/*--------------------------------------------------------------------------*/
/* Ceiling function (NOTE: This function is nondifferentiable) */
adub ceil(const badouble &x) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  locint locat = next_loc();

  double coval =
      ADOLC_MATH_NSP::ceil(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

  if (ADOLC_CURRENT_TAPE_INFOS
          .traceFlag) { // old:
                        // write_args_d_a(ceil_op,locat,coval,a.tape_loc_.loc_);
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif
      put_op(ceil_op);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
      ADOLC_PUT_LOCINT(locat);            // = res
      ADOLC_PUT_VAL(coval);               // = coval

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[locat]);
#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[locat]) {
      if (coval == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(locat);
      } else if (coval == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(locat);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(locat);
        ADOLC_PUT_VAL(coval);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[locat]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[locat] = coval;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[locat] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif
  return locat;
}

/*--------------------------------------------------------------------------*/
/* Floor function (NOTE: This function is nondifferentiable) */
adub floor(const badouble &x) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  locint locat = next_loc();

  double coval =
      ADOLC_MATH_NSP::floor(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

  if (ADOLC_CURRENT_TAPE_INFOS
          .traceFlag) { // old:
                        // write_args_d_a(floor_op,locat,coval,a.tape_loc_.loc_);
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif
      put_op(floor_op);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg
      ADOLC_PUT_LOCINT(locat);            // = res
      ADOLC_PUT_VAL(coval);               // = coval

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[locat]);
#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[locat]) {
      if (coval == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(locat);
      } else if (coval == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(locat);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(locat);
        ADOLC_PUT_VAL(coval);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[locat]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[locat] = coval;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[locat] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif
  return locat;
}

/*--------------------------------------------------------------------------*/
/* Asinh value -- really a quadrature */
adub asinh(const badouble &x) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  locint locat = next_loc();
  double coval =
      ADOLC_MATH_NSP_ERF::asinh(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

  adouble y = 1.0 / sqrt(1.0 + x * x);

  if (ADOLC_CURRENT_TAPE_INFOS
          .traceFlag) { // old:
                        // write_quad(asinh_op,locat,a.tape_loc_.loc_,b.tape_loc_.loc_);
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS
            .actStore[a.tape_loc_.loc_]) { // y will have same activity as x and
                                           // can be
                                           // considered as second input here
#endif
      put_op(asinh_op);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg1
      ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = arg2
      ADOLC_PUT_LOCINT(locat);            // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[locat]);
#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[locat]) {
      if (coval == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(locat);
      } else if (coval == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(locat);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(locat);
        ADOLC_PUT_VAL(coval);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[locat]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[locat] = coval;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[locat] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif
  ADOLC_OPENMP_RESTORE_THREAD_NUMBER;
  return locat;
}

/*--------------------------------------------------------------------------*/
/* Acosh value -- really a quadrature */
adub acosh(const badouble &x) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  locint locat = next_loc();
  double coval =
      ADOLC_MATH_NSP_ERF::acosh(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

  adouble y = 1.0 / sqrt(x * x - 1.0);

  if (ADOLC_CURRENT_TAPE_INFOS
          .traceFlag) { // old:
                        // write_quad(acosh_op,locat,a.tape_loc_.loc_,b.tape_loc_.loc_);
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS
            .actStore[a.tape_loc_.loc_]) { // y will have same activity as x and
                                           // can be
                                           // considered as second input here
#endif
      put_op(acosh_op);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg1
      ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = arg2
      ADOLC_PUT_LOCINT(locat);            // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[locat]);
#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[locat]) {
      if (coval == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(locat);
      } else if (coval == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(locat);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(locat);
        ADOLC_PUT_VAL(coval);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[locat]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[locat] = coval;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[locat] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif
  ADOLC_OPENMP_RESTORE_THREAD_NUMBER;
  return locat;
}

/*--------------------------------------------------------------------------*/
/* Atanh value -- really a quadrature */
adub atanh(const badouble &x) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  locint locat = next_loc();
  double coval =
      ADOLC_MATH_NSP_ERF::atanh(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

  adouble y = 1.0 / (1.0 - x * x);

  if (ADOLC_CURRENT_TAPE_INFOS
          .traceFlag) { // old:
                        // write_quad(atanh_op,locat,a.tape_loc_.loc_,b.tape_loc_.loc_);
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS
            .actStore[a.tape_loc_.loc_]) { // y will have same activity as x and
                                           // can be
                                           // considered as second input here
#endif
      put_op(atanh_op);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg1
      ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = arg2
      ADOLC_PUT_LOCINT(locat);            // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[locat]);
#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[locat]) {
      if (coval == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(locat);
      } else if (coval == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(locat);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(locat);
        ADOLC_PUT_VAL(coval);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[locat]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[locat] = coval;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[locat] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif
  ADOLC_OPENMP_RESTORE_THREAD_NUMBER;
  return locat;
}

/*--------------------------------------------------------------------------*/
/*  The error function erf */
adub erf(const badouble &x) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  locint locat = next_loc();
  double coval =
      ADOLC_MATH_NSP_ERF::erf(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

  adouble y =
      2.0 / ADOLC_MATH_NSP_ERF::sqrt(ADOLC_MATH_NSP::acos(-1.0)) * exp(-x * x);

  if (ADOLC_CURRENT_TAPE_INFOS
          .traceFlag) { // old:
                        // write_quad(erf_op,locat,a.tape_loc_.loc_,b.tape_loc_.loc_);
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS
            .actStore[a.tape_loc_.loc_]) { // y will have same activity as x and
                                           // can be
                                           // considered as second input here
#endif
      put_op(erf_op);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg1
      ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = arg2
      ADOLC_PUT_LOCINT(locat);            // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[locat]);
#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[locat]) {
      if (coval == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(locat);
      } else if (coval == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(locat);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(locat);
        ADOLC_PUT_VAL(coval);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[locat]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[locat] = coval;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[locat] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif
  ADOLC_OPENMP_RESTORE_THREAD_NUMBER;
  return locat;
}

adub erfc(const badouble &x) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  locint locat = next_loc();
  double coval =
      ADOLC_MATH_NSP_ERF::erfc(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);

  adouble y =
      -2.0 / ADOLC_MATH_NSP_ERF::sqrt(ADOLC_MATH_NSP::acos(-1.0)) * exp(-x * x);

  if (ADOLC_CURRENT_TAPE_INFOS
          .traceFlag) { // old:
                        // write_quad(erfc_op,locat,a.tape_loc_.loc_,b.tape_loc_.loc_);
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS
            .actStore[a.tape_loc_.loc_]) { // y will have same activity as x and
                                           // can be
                                           // considered as second input here
#endif
      put_op(erfc_op);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg1
      ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = arg2
      ADOLC_PUT_LOCINT(locat);            // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[locat]);
#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[locat]) {
      if (coval == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(locat);
      } else if (coval == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(locat);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(locat);
        ADOLC_PUT_VAL(coval);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[locat]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[locat] = coval;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[locat] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif
  ADOLC_OPENMP_RESTORE_THREAD_NUMBER;
  return locat;
}

/*--------------------------------------------------------------------------*/
/* Fabs Function (NOTE: This function is also nondifferentiable at x=0) */
adub fabs(const badouble &x) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  locint locat = next_loc();

  double coval = 1.0;
  double temp =
      ADOLC_MATH_NSP::fabs(ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]);
  if (temp != ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_])
    coval = 0.0;

  if (ADOLC_CURRENT_TAPE_INFOS
          .traceFlag) { /*  write_args_d_a(abs_val,locat,coval,a.tape_loc_.loc_);
                         */
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
#endif
      put_op(abs_val);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); /* arg */
      ADOLC_PUT_LOCINT(locat);            /* res */
      ADOLC_PUT_VAL(coval);               /* coval */

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.stats[NO_MIN_MAX])
        ++ADOLC_CURRENT_TAPE_INFOS.numSwitches;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[locat]);
#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[locat]) {
      if (temp == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(locat);
      } else if (temp == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(locat);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(locat);
        ADOLC_PUT_VAL(temp);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[locat]);
    }
#endif
  }
  ADOLC_GLOBAL_TAPE_VARS.store[locat] = temp;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[locat] =
      ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_];
#endif
  return locat;
}

/*--------------------------------------------------------------------------*/
/* max and min functions  (changed : 11/15/95) */
adub fmin(const badouble &x,
          const badouble
              &y) { /* olvo 980702 tested: return 0.5*fabs(x+y-fabs(x-y)); */
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  if (ADOLC_CURRENT_TAPE_INFOS.stats[NO_MIN_MAX])
    return ((x + y - fabs(x - y)) / 2.0);

#if defined(ADOLC_TRACK_ACTIVITY)
  if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_] &&
        !ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_]) {
      locint tmploc = a.tape_loc_.loc_;
      double temp = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
      if (temp == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(tmploc);
      } else if (temp == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(tmploc);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(tmploc);
        ADOLC_PUT_VAL(temp);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tmploc]);
    }
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] &&
        !ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]) {
      locint tmploc = b.tape_loc_.loc_;
      double temp = ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];
      if (temp == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(tmploc);
      } else if (temp == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(tmploc);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(tmploc);
        ADOLC_PUT_VAL(temp);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tmploc]);
    }
  }
#endif

  locint locat = next_loc();
  double coval, temp;

  if (ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_] <
      ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_]) {
    coval = 0.0;
    temp = ADOLC_GLOBAL_TAPE_VARS.store[b.tape_loc_.loc_];
  } else {
    coval = 1.0;
    temp = ADOLC_GLOBAL_TAPE_VARS.store[a.tape_loc_.loc_];
  }

  if (ADOLC_CURRENT_TAPE_INFOS
          .traceFlag) { // old:
                        // write_min_op(a.tape_loc_.loc_,b.tape_loc_.loc_,locat,0.0);
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] ||
        ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]) {
#endif
      put_op(min_op);
      ADOLC_PUT_LOCINT(a.tape_loc_.loc_); // = arg1
      ADOLC_PUT_LOCINT(b.tape_loc_.loc_); // = arg2
      ADOLC_PUT_LOCINT(locat);            // = res
      ADOLC_PUT_VAL(coval);               // = coval

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[locat]);
#if defined(ADOLC_TRACK_ACTIVITY)
    } else if (ADOLC_GLOBAL_TAPE_VARS.actStore[locat]) {
      if (temp == 0.0) {
        put_op(assign_d_zero);
        ADOLC_PUT_LOCINT(locat);
      } else if (temp == 1.0) {
        put_op(assign_d_one);
        ADOLC_PUT_LOCINT(locat);
      } else {
        put_op(assign_d);
        ADOLC_PUT_LOCINT(locat);
        ADOLC_PUT_VAL(temp);
      }

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[locat]);
    }
#endif
  }

  ADOLC_GLOBAL_TAPE_VARS.store[locat] = temp;
#if defined(ADOLC_TRACK_ACTIVITY)
  ADOLC_GLOBAL_TAPE_VARS.actStore[locat] =
      (ADOLC_GLOBAL_TAPE_VARS.actStore[a.tape_loc_.loc_] ||
       ADOLC_GLOBAL_TAPE_VARS.actStore[b.tape_loc_.loc_]);
#endif
  return locat;
}

/*--------------------------------------------------------------------------*/
/*21.8.96*/
adub fmin(double d, const badouble &a {
  adouble x = d;
  return (fmin(x, a);
}

/*--------------------------------------------------------------------------*/
adub fmin(const badouble &x, double d) {
  adouble y = d;
  return (fmin(x, y));
}

/*--------------------------------------------------------------------------*/
adub fmax(const badouble &x, const badouble &y) {
  return (-fmin(-x, -y)); }

/*--------------------------------------------------------------------------*/
/*21.8.96*/
adub fmax(double d, const badouble &y) {
  adouble x = d;
  return (-fmin(-x, -y));
}

/*--------------------------------------------------------------------------*/
adub fmax(const badouble &x, double d) {
  adouble y = d;
  return (-fmin(-x, -y));
}

/*--------------------------------------------------------------------------*/
/* Ldexp Function */
adub ldexp(const badouble &x, int exp) {
  return x * ldexp(1.0, exp); }
/*--------------------------------------------------------------------------*/
/* frexp Function */
adub frexp(const badouble &x, int *n) {
  double v = frexp(x.value(), n);
  adouble r = x - v;
  adouble z = r - double(*n);
  if (z == 0) {
    return (x - double(*n));
  } else {
    fprintf(stderr,
            "ADOL-C warning: std::frexp() returned inconsistent results\n");
    return (r - double(*n));
  }
}

/*--------------------------------------------------------------------------*/
/* Macro for user defined quadratures, example myquad is below.*/
/* the forward sweep tests if the tape is executed exactly at  */
/* the same argument point otherwise it stops with a returnval */
#define extend_quad(func, integrand)                                           \
  adouble func(const badouble &arg) {                                          \
    adouble temp;                                                              \
    adouble val;                                                               \
    integrand;                                                                 \
    ADOLC_OPENMP_THREAD_NUMBER;                                                \
    ADOLC_OPENMP_GET_THREAD_NUMBER;                                            \
    if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {                                  \
      put_op(gen_quad);                                                        \
      ADOLC_PUT_LOCINT(arg.loc());                                             \
      ADOLC_PUT_LOCINT(val.loc());                                             \
      ADOLC_PUT_LOCINT(temp.loc());                                            \
      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;                                 \
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)                                \
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[temp.loc()]);         \
    }                                                                          \
    ADOLC_GLOBAL_TAPE_VARS.store[temp.loc()] =                                 \
        func(ADOLC_GLOBAL_TAPE_VARS.store[arg.loc()]);                         \
    if (ADOLC_CURRENT_TAPE_INFOS.traceFlag) {                                  \
      ADOLC_PUT_VAL(ADOLC_GLOBAL_TAPE_VARS.store[arg.loc()]);                  \
      ADOLC_PUT_VAL(ADOLC_GLOBAL_TAPE_VARS.store[temp.loc()]);                 \
    }                                                                          \
    return temp;                                                               \
  }

double myquad(double &x) {
  double res;
  res = ADOLC_MATH_NSP::log(x);
  return res;
}

/* This defines the natural logarithm as a quadrature */

extend_quad(myquad, val = 1 / arg);

void condassign(adouble &res, const badouble &cond, const badouble &arg1,
                const badouble &arg2) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  if (ADOLC_CURRENT_TAPE_INFOS
          .traceFlag) { // old:
                        // write_condassign(res.loc(),cond.loc(),arg1.loc(),
                        //		     arg2.loc());
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[cond.loc()]) {
      if (!ADOLC_GLOBAL_TAPE_VARS.actStore[arg1.loc()]) {
        locint tmploc = arg1.loc();
        double temp = ADOLC_GLOBAL_TAPE_VARS.store[arg1.loc()];
        if (temp == 0.0) {
          put_op(assign_d_zero);
          ADOLC_PUT_LOCINT(tmploc);
        } else if (temp == 1.0) {
          put_op(assign_d_one);
          ADOLC_PUT_LOCINT(tmploc);
        } else {
          put_op(assign_d);
          ADOLC_PUT_LOCINT(tmploc);
          ADOLC_PUT_VAL(temp);
        }

        ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
        if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
          ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tmploc]);
      }
      if (!ADOLC_GLOBAL_TAPE_VARS.actStore[arg2.loc()]) {
        locint tmploc = arg2.loc();
        double temp = ADOLC_GLOBAL_TAPE_VARS.store[arg2.loc()];
        if (temp == 0.0) {
          put_op(assign_d_zero);
          ADOLC_PUT_LOCINT(tmploc);
        } else if (temp == 1.0) {
          put_op(assign_d_one);
          ADOLC_PUT_LOCINT(tmploc);
        } else {
          put_op(assign_d);
          ADOLC_PUT_LOCINT(tmploc);
          ADOLC_PUT_VAL(temp);
        }

        ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
        if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
          ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tmploc]);
      }
#endif
      put_op(cond_assign);
      ADOLC_PUT_LOCINT(cond.loc()); // = arg
      ADOLC_PUT_VAL(ADOLC_GLOBAL_TAPE_VARS.store[cond.loc()]);
      ADOLC_PUT_LOCINT(arg1.loc()); // = arg1
      ADOLC_PUT_LOCINT(arg2.loc()); // = arg2
      ADOLC_PUT_LOCINT(res.loc());  // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[res.loc()]);
#if defined(ADOLC_TRACK_ACTIVITY)
    } else {
      locint x_loc;
      if (ADOLC_GLOBAL_TAPE_VARS.store[cond.loc()] > 0)
        x_loc = arg1.loc();
      else
        x_loc = arg2.loc();

      if (ADOLC_GLOBAL_TAPE_VARS.actStore[x_loc]) {
        put_op(assign_a);
        ADOLC_PUT_LOCINT(x_loc);     // = arg
        ADOLC_PUT_LOCINT(res.loc()); // = res

        ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
        if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
          ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[res.loc()]);
      } else {
        if (ADOLC_GLOBAL_TAPE_VARS.actStore[res.loc()]) {
          double coval = ADOLC_GLOBAL_TAPE_VARS.store[x_loc];
          if (coval == 0) {
            put_op(assign_d_zero);
            ADOLC_PUT_LOCINT(res.loc()); // = res
          } else if (coval == 1.0) {
            put_op(assign_d_one);
            ADOLC_PUT_LOCINT(res.loc()); // = res
          } else {
            put_op(assign_d);
            ADOLC_PUT_LOCINT(res.loc()); // = res
            ADOLC_PUT_VAL(coval);        // = coval
          }

          ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
          if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
            ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[res.loc()]);
        }
      }
    }
#endif
  }

  if (ADOLC_GLOBAL_TAPE_VARS.store[cond.loc()] > 0)
    ADOLC_GLOBAL_TAPE_VARS.store[res.loc()] =
        ADOLC_GLOBAL_TAPE_VARS.store[arg1.loc()];
  else
    ADOLC_GLOBAL_TAPE_VARS.store[res.loc()] =
        ADOLC_GLOBAL_TAPE_VARS.store[arg2.loc()];
#if defined(ADOLC_TRACK_ACTIVITY)
  if (!ADOLC_GLOBAL_TAPE_VARS.actStore[cond.loc()]) {
    if (ADOLC_GLOBAL_TAPE_VARS.store[cond.loc()] > 0)
      ADOLC_GLOBAL_TAPE_VARS.actStore[res.loc()] =
          ADOLC_GLOBAL_TAPE_VARS.actStore[arg1.loc()];
    else
      ADOLC_GLOBAL_TAPE_VARS.actStore[res.loc()] =
          ADOLC_GLOBAL_TAPE_VARS.actStore[arg2.loc()];
  } else
    ADOLC_GLOBAL_TAPE_VARS.actStore[res.loc()] =
        ADOLC_GLOBAL_TAPE_VARS.actStore[cond.loc()];
#endif
}

/*--------------------------------------------------------------------------*/
void condassign(adouble &res, const badouble &cond, const badouble &arg) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  if (ADOLC_CURRENT_TAPE_INFOS
          .traceFlag) { // old:
                        // write_condassign2(res.loc(),cond.loc(),arg.loc());
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[cond.loc()]) {
      if (!ADOLC_GLOBAL_TAPE_VARS.actStore[arg.loc()]) {
        locint tmploc = arg.loc();
        double temp = ADOLC_GLOBAL_TAPE_VARS.store[arg.loc()];
        if (temp == 0.0) {
          put_op(assign_d_zero);
          ADOLC_PUT_LOCINT(tmploc);
        } else if (temp == 1.0) {
          put_op(assign_d_one);
          ADOLC_PUT_LOCINT(tmploc);
        } else {
          put_op(assign_d);
          ADOLC_PUT_LOCINT(tmploc);
          ADOLC_PUT_VAL(temp);
        }

        ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
        if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
          ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tmploc]);
      }
#endif
      put_op(cond_assign_s);
      ADOLC_PUT_LOCINT(cond.loc()); // = arg
      ADOLC_PUT_VAL(ADOLC_GLOBAL_TAPE_VARS.store[cond.loc()]);
      ADOLC_PUT_LOCINT(arg.loc()); // = arg1
      ADOLC_PUT_LOCINT(res.loc()); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[res.loc()]);
#if defined(ADOLC_TRACK_ACTIVITY)
    } else {
      locint x_loc;
      if (ADOLC_GLOBAL_TAPE_VARS.store[cond.loc()] > 0) {
        x_loc = arg.loc();
        if (ADOLC_GLOBAL_TAPE_VARS.actStore[x_loc]) {
          put_op(assign_a);
          ADOLC_PUT_LOCINT(x_loc);     // = arg
          ADOLC_PUT_LOCINT(res.loc()); // = res

          ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
          if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
            ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[res.loc()]);
        } else {
          if (ADOLC_GLOBAL_TAPE_VARS.actStore[res.loc()]) {
            double coval = ADOLC_GLOBAL_TAPE_VARS.store[x_loc];
            if (coval == 0) {
              put_op(assign_d_zero);
              ADOLC_PUT_LOCINT(res.loc()); // = res
            } else if (coval == 1.0) {
              put_op(assign_d_one);
              ADOLC_PUT_LOCINT(res.loc()); // = res
            } else {
              put_op(assign_d);
              ADOLC_PUT_LOCINT(res.loc()); // = res
              ADOLC_PUT_VAL(coval);        // = coval
            }

            ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
            if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
              ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[res.loc()]);
          }
        }
      }
    }
#endif
  }

  if (ADOLC_GLOBAL_TAPE_VARS.store[cond.loc()] > 0)
    ADOLC_GLOBAL_TAPE_VARS.store[res.loc()] =
        ADOLC_GLOBAL_TAPE_VARS.store[arg.loc()];
#if defined(ADOLC_TRACK_ACTIVITY)
  if (!ADOLC_GLOBAL_TAPE_VARS.actStore[cond.loc()]) {
    if (ADOLC_GLOBAL_TAPE_VARS.store[cond.loc()] > 0)
      ADOLC_GLOBAL_TAPE_VARS.actStore[res.loc()] =
          ADOLC_GLOBAL_TAPE_VARS.actStore[arg.loc()];
  } else
    ADOLC_GLOBAL_TAPE_VARS.actStore[res.loc()] =
        ADOLC_GLOBAL_TAPE_VARS.actStore[cond.loc()];
#endif
}
/*--------------------------------------------------------------------------*/
void condeqassign(adouble &res, const badouble &cond, const badouble &arg1,
                  const badouble &arg2) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  if (ADOLC_CURRENT_TAPE_INFOS
          .traceFlag) { // old:
                        // write_condassign(res.loc(),cond.loc(),arg1.loc(),
                        //		     arg2.loc());
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[cond.loc()]) {
      if (!ADOLC_GLOBAL_TAPE_VARS.actStore[arg1.loc()]) {
        locint tmploc = arg1.loc();
        double temp = ADOLC_GLOBAL_TAPE_VARS.store[arg1.loc()];
        if (temp == 0.0) {
          put_op(assign_d_zero);
          ADOLC_PUT_LOCINT(tmploc);
        } else if (temp == 1.0) {
          put_op(assign_d_one);
          ADOLC_PUT_LOCINT(tmploc);
        } else {
          put_op(assign_d);
          ADOLC_PUT_LOCINT(tmploc);
          ADOLC_PUT_VAL(temp);
        }

        ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
        if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
          ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tmploc]);
      }
      if (!ADOLC_GLOBAL_TAPE_VARS.actStore[arg2.loc()]) {
        locint tmploc = arg2.loc();
        double temp = ADOLC_GLOBAL_TAPE_VARS.store[arg2.loc()];
        if (temp == 0.0) {
          put_op(assign_d_zero);
          ADOLC_PUT_LOCINT(tmploc);
        } else if (temp == 1.0) {
          put_op(assign_d_one);
          ADOLC_PUT_LOCINT(tmploc);
        } else {
          put_op(assign_d);
          ADOLC_PUT_LOCINT(tmploc);
          ADOLC_PUT_VAL(temp);
        }

        ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
        if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
          ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tmploc]);
      }
#endif
      put_op(cond_eq_assign);
      ADOLC_PUT_LOCINT(cond.loc()); // = arg
      ADOLC_PUT_VAL(ADOLC_GLOBAL_TAPE_VARS.store[cond.loc()]);
      ADOLC_PUT_LOCINT(arg1.loc()); // = arg1
      ADOLC_PUT_LOCINT(arg2.loc()); // = arg2
      ADOLC_PUT_LOCINT(res.loc());  // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[res.loc()]);
#if defined(ADOLC_TRACK_ACTIVITY)
    } else {
      locint x_loc;
      if (ADOLC_GLOBAL_TAPE_VARS.store[cond.loc()] > 0)
        x_loc = arg1.loc();
      else
        x_loc = arg2.loc();

      if (ADOLC_GLOBAL_TAPE_VARS.actStore[x_loc]) {
        put_op(assign_a);
        ADOLC_PUT_LOCINT(x_loc);     // = arg
        ADOLC_PUT_LOCINT(res.loc()); // = res

        ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
        if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
          ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[res.loc()]);
      } else {
        if (ADOLC_GLOBAL_TAPE_VARS.actStore[res.loc()]) {
          double coval = ADOLC_GLOBAL_TAPE_VARS.store[x_loc];
          if (coval == 0) {
            put_op(assign_d_zero);
            ADOLC_PUT_LOCINT(res.loc()); // = res
          } else if (coval == 1.0) {
            put_op(assign_d_one);
            ADOLC_PUT_LOCINT(res.loc()); // = res
          } else {
            put_op(assign_d);
            ADOLC_PUT_LOCINT(res.loc()); // = res
            ADOLC_PUT_VAL(coval);        // = coval
          }

          ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
          if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
            ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[res.loc()]);
        }
      }
    }
#endif
  }

  if (ADOLC_GLOBAL_TAPE_VARS.store[cond.loc()] >= 0)
    ADOLC_GLOBAL_TAPE_VARS.store[res.loc()] =
        ADOLC_GLOBAL_TAPE_VARS.store[arg1.loc()];
  else
    ADOLC_GLOBAL_TAPE_VARS.store[res.loc()] =
        ADOLC_GLOBAL_TAPE_VARS.store[arg2.loc()];
#if defined(ADOLC_TRACK_ACTIVITY)
  if (!ADOLC_GLOBAL_TAPE_VARS.actStore[cond.loc()]) {
    if (ADOLC_GLOBAL_TAPE_VARS.store[cond.loc()] > 0)
      ADOLC_GLOBAL_TAPE_VARS.actStore[res.loc()] =
          ADOLC_GLOBAL_TAPE_VARS.actStore[arg1.loc()];
    else
      ADOLC_GLOBAL_TAPE_VARS.actStore[res.loc()] =
          ADOLC_GLOBAL_TAPE_VARS.actStore[arg2.loc()];
  } else
    ADOLC_GLOBAL_TAPE_VARS.actStore[res.loc()] =
        ADOLC_GLOBAL_TAPE_VARS.actStore[cond.loc()];
#endif
}

/*--------------------------------------------------------------------------*/
void condeqassign(adouble &res, const badouble &cond, const badouble &arg) {
  ADOLC_OPENMP_THREAD_NUMBER;
  ADOLC_OPENMP_GET_THREAD_NUMBER;
  if (ADOLC_CURRENT_TAPE_INFOS
          .traceFlag) { // old:
                        // write_condassign2(res.loc(),cond.loc(),arg.loc());
#if defined(ADOLC_TRACK_ACTIVITY)
    if (ADOLC_GLOBAL_TAPE_VARS.actStore[cond.loc()]) {
      if (!ADOLC_GLOBAL_TAPE_VARS.actStore[arg.loc()]) {
        locint tmploc = arg.loc();
        double temp = ADOLC_GLOBAL_TAPE_VARS.store[arg.loc()];
        if (temp == 0.0) {
          put_op(assign_d_zero);
          ADOLC_PUT_LOCINT(tmploc);
        } else if (temp == 1.0) {
          put_op(assign_d_one);
          ADOLC_PUT_LOCINT(tmploc);
        } else {
          put_op(assign_d);
          ADOLC_PUT_LOCINT(tmploc);
          ADOLC_PUT_VAL(temp);
        }

        ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
        if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
          ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[tmploc]);
      }
#endif
      put_op(cond_eq_assign_s);
      ADOLC_PUT_LOCINT(cond.loc()); // = arg
      ADOLC_PUT_VAL(ADOLC_GLOBAL_TAPE_VARS.store[cond.loc()]);
      ADOLC_PUT_LOCINT(arg.loc()); // = arg1
      ADOLC_PUT_LOCINT(res.loc()); // = res

      ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
      if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
        ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[res.loc()]);
#if defined(ADOLC_TRACK_ACTIVITY)
    } else {
      locint x_loc;
      if (ADOLC_GLOBAL_TAPE_VARS.store[cond.loc()] > 0) {
        x_loc = arg.loc();
        if (ADOLC_GLOBAL_TAPE_VARS.actStore[x_loc]) {
          put_op(assign_a);
          ADOLC_PUT_LOCINT(x_loc);     // = arg
          ADOLC_PUT_LOCINT(res.loc()); // = res

          ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
          if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
            ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[res.loc()]);
        } else {
          if (ADOLC_GLOBAL_TAPE_VARS.actStore[res.loc()]) {
            double coval = ADOLC_GLOBAL_TAPE_VARS.store[x_loc];
            if (coval == 0) {
              put_op(assign_d_zero);
              ADOLC_PUT_LOCINT(res.loc()); // = res
            } else if (coval == 1.0) {
              put_op(assign_d_one);
              ADOLC_PUT_LOCINT(res.loc()); // = res
            } else {
              put_op(assign_d);
              ADOLC_PUT_LOCINT(res.loc()); // = res
              ADOLC_PUT_VAL(coval);        // = coval
            }

            ++ADOLC_CURRENT_TAPE_INFOS.numTays_Tape;
            if (ADOLC_CURRENT_TAPE_INFOS.keepTaylors)
              ADOLC_WRITE_SCAYLOR(ADOLC_GLOBAL_TAPE_VARS.store[res.loc()]);
          }
        }
      }
    }
#endif
  }

  if (ADOLC_GLOBAL_TAPE_VARS.store[cond.loc()] >= 0)
    ADOLC_GLOBAL_TAPE_VARS.store[res.loc()] =
        ADOLC_GLOBAL_TAPE_VARS.store[arg.loc()];
#if defined(ADOLC_TRACK_ACTIVITY)
  if (!ADOLC_GLOBAL_TAPE_VARS.actStore[cond.loc()]) {
    if (ADOLC_GLOBAL_TAPE_VARS.store[cond.loc()] > 0)
      ADOLC_GLOBAL_TAPE_VARS.actStore[res.loc()] =
          ADOLC_GLOBAL_TAPE_VARS.actStore[arg.loc()];
  } else
    ADOLC_GLOBAL_TAPE_VARS.actStore[res.loc()] =
        ADOLC_GLOBAL_TAPE_VARS.actStore[cond.loc()];
#endif
}

/****************************************************************************/
/*                                                                THAT'S ALL*/
