/*
 * helloworld - simple hello world kernel module.
 * Copyright (c) 2017, Sebastien Vincent
 *
 * Distributed under the terms of the BSD 3-clause License.
 * See the LICENSE file for details.
 */

/**
 * \file helloworld.c
 * \brief Hello world kernel module for NetBSD.
 * \author Sebastien Vincent
 * \date 2017
 */

#include <sys/types.h>
#include <sys/module.h>
#include <sys/systm.h>
#include <sys/param.h>
#include <sys/kernel.h>

/**
 * \brief Name of the module.
 */
static char name[1024] = "helloworld";

/**
 * \brief String value.
 */
static char value[1024] = "helloworld";

/**
 * \brief Cookie value.
 */
static int cookie = 0;

/**
 * \brief Handle the user parameters.
 * \param props dictionary of key/value pairs.
 */
static void helloworld_handle_props(prop_dictionary_t props)
{
	if(props != NULL)
	{
    prop_string_t str;

    /* name parameter */
    str = prop_dictionary_get(props, "value");
    if(str)
    {
      const char* n = prop_string_cstring_nocopy(str);
      strcpy(value, n);
    }

    /* cookie parameter */
    prop_dictionary_get_int32(props, "cookie", &cookie);
	}
}

/**
 * \brief Module loader.
 * \param cmd command event (load, unload...).
 * \param args arguments.
 * \return 0 if success, error code otherwise.
 */
static int helloworld_modcmd(modcmd_t cmd, void* args)
{
  int err = 0;

  switch(cmd)
  {
  case MODULE_CMD_INIT:
    /* parse arguments */
    helloworld_handle_props(args);

    printf("%s: initialization\n", name);
    printf("%s: value=%s cookie=%d\n", name, value, cookie);
    break;
  case MODULE_CMD_FINI:
    printf("%s: finalization\n", name);
    break;
  case MODULE_CMD_STAT:
    break;
  default:
    err = ENOTTY;
    break;
  }

  return err;
}

MODULE(MODULE_CLASS_MISC, helloworld, NULL);
