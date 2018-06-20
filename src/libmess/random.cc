/*
    Copyright (C) 2018 Yuri Georgievski (ygeorgi@anl.gov), Stephen J.
    Klippenstein (sjk@anl.gov), and Argonne National Laboratory.

    See https://github.com/PACChem/MESS for copyright and licensing details.
*/

#include "random.hh"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

/****************************************************************
 ******************* Random Number Generators *******************
 ****************************************************************/

void Random::init ()
{
  srand48(std::time(0));
}

void Random::init (int i)
{
  srand48(i);
}

double Random::flat ()
{
    return drand48 ();
}

double Random::norm ()
{// normal distribution RNG

  static bool first = true;
  static double x1, x2;
  double r;
  if (first)
    {
      first = false;
      do
	{
	  x1 = 2.0 * Random::flat() - 1.0;
	  x2 = 2.0 * Random::flat() - 1.0;
	  r = x1 * x1 + x2 * x2;
	}
      while (r > 1.0);
      r = std::sqrt(-2.0 * std::log(r)/r);
      x1 *= r;
      x2 *= r;
      return x1;
    }
  else
    {
      first = true;
      return x2;
    }
}

double Random::exp ()
{// RNG for the distribution exp(-u**2/2)u*du
  return std::sqrt(-2.0 * std::log(Random::flat()));
}

void Random::orient (double* vec, int dim)
{ // randomly orients vector _vec_ of the dimension _dim_ of unity length
  double norm, temp;
  do {
    norm = 0.0;
    for (int i = 0; i < dim; ++i)
      {
	temp = 2.0 * Random::flat () - 1.0;
	norm += temp * temp;
	vec[i] = temp;
      }
  } while (norm >= 1.0 || norm < 0.0001);

  norm = std::sqrt(norm);
  for (int i = 0; i < dim; ++i)
    vec[i] /= norm;
}

double Random::vol (double* vec, int dim)
{ // random point inside the sphere of unity radius
  double norm, temp;
  do {
    norm = 0.0;
    for (int i = 0; i < dim; ++i) {
      temp = 2.0 * Random::flat () - 1.0;
      norm += temp * temp;
      vec[i] = temp;
    }
  } while (norm > 1.0);

  return norm;
}

// random point inside the spherical layer 
void Random::spherical_layer (double* vec, int dim, double rmin, double rmax) throw(Error::General)
{ 
  const char funame [] = "Random::spherical_layer: ";

  if(rmin <= 0. || rmax <= 0. || rmin >= rmax || dim <= 0 || vec == 0) {
    std::cerr << funame << "out of range\n";
    throw Error::Range();
  }
    
  double norm, dtemp;
  
  const double rmin2 = rmin * rmin;
  const double rmax2 = rmax * rmax;

  double* const end = vec + dim;

  do {
    norm = 0.;
    for (double* it = vec; it != end; ++it) {
      dtemp = (2. * Random::flat () - 1.) * rmax;
      *it = dtemp;
      norm += dtemp * dtemp;
    }
  } while (norm < rmin2 || norm > rmax2);
}
