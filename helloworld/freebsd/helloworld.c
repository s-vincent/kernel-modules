/*
 * helloworld - simple hello world FreeBSD kernel module.
 * Copyright (c) 2016, Sebastien Vincent
 *
 * Distributed under the terms of the BSD 3-clause License.
 * See the LICENSE file for details.
 */

/**
 * \file helloworld.c
 * \brief Hello world kernel module for FreeBSD.
 * \author Sebastien Vincent
 * \date 2016
 */

#include <sys/types.h>
#include <sys/module.h>
#include <sys/systm.h>
#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/kenv.h>

/**
 * \brief Name of the module.
 */
static char name[1024] = "helloworld";

/**
 * \brief Cookie value.
 */
static int cookie = 0;

/**
 * \brief Module loader.
 * \param m module pointer.
 * \param evt event (load, unload...).
 * \param arg additionnal argument.
 * \return 0 if success, error code otherwise.
 */
static int helloworld_loader(struct module* m, int evt, void* arg)
{
  int err = 0;

  switch(evt)
  {
  case MOD_LOAD:
    TUNABLE_STR_FETCH("helloworld.name", name, sizeof(name));
    TUNABLE_INT_FETCH("helloworld.cookie", &cookie);
    uprintf("%s.%d: initialization\n", name, cookie);
    break;
  case MOD_UNLOAD:
    uprintf("%s.%d: finalization\n", name, cookie);
    break;
  default:
    err = EOPNOTSUPP;
    break;
  }

  return err;
}

/**
 * \brief Module structure for kernel registration.
 */
static moduledata_t helloworld_mod = 
{
  "helloworld", /* name of the module */
  helloworld_loader, /* callback for event */
  NULL /* argument */
};

DECLARE_MODULE(helloworld, helloworld_mod, SI_SUB_KLD, SI_ORDER_ANY);

