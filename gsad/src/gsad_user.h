/* Greenbone Security Assistant
 * $Id$
 * Description: GSAD user handling
 *
 * Authors:
 * Björn Ricks <bjoern.ricks@greenbone.net>
 *
 * Copyright:
 * Copyright (C) 2016 Greenbone Networks GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _GSAD_USER_H
#define _GSAD_USER_H

#include <glib.h>

#include "gsad_params.h"

#define USER_OK 0
#define USER_BAD_TOKEN 1
#define USER_EXPIRED_TOKEN 2
#define USER_BAD_MISSING_COOKIE 3
#define USER_BAD_MISSING_TOKEN 4
#define USER_GUEST_LOGIN_FAILED 5
#define USER_GMP_DOWN 6
#define USER_IP_ADDRESS_MISSMATCH 7
#define USER_GUEST_LOGIN_ERROR -1

/**
 *  @brief Structure of credential related information.
 */
typedef struct
{
  struct timeval cmd_start; ///< Seconds since command page handler started.
  char *username;     ///< Name of user.
  char *password;     ///< User's password.
  char *role;         ///< User's role.
  char *timezone;     ///< User's timezone.
  char *token;        ///< Session token.
  char *caller;       ///< Caller URL, for POST relogin.
  char *current_page; ///< Current page URL, for refresh.
  char *capabilities; ///< Capabilites of manager.
  char *language;     ///< Accept-Language browser header.
  char *severity;     ///< Severity class.
  char *pw_warning;   ///< Password policy warning message
  char *client_address; ///< Client's address.
  GTree *last_filt_ids; ///< Last filter ids.
  params_t *params;   ///< Request parameters.
  int guest;          ///< Whether the user is a guest user.
  char *sid;          ///< Session ID of the user.
} credentials_t;

/**
 * @brief User information type, for sessions.
 */
typedef struct user user_t;

int user_find (const gchar *cookie, const gchar *token, const char *address,
               user_t **user_return);

void user_remove (user_t *user);

void user_free (user_t *user);

user_t *
user_add (const gchar *username, const gchar *password, const gchar *timezone,
          const gchar *severity, const gchar *role, const gchar *capabilities,
          const gchar *language, const gchar *pw_warning, const char *address);

int user_set_timezone (const gchar *token, const gchar *timezone);

int user_set_password (const gchar *token, const gchar *password);

int user_set_severity (const gchar *token, const gchar *severity);

int user_set_language (const gchar *token, const gchar *language);

int user_logout_all_sessions (const gchar *username,
                              credentials_t *credentials);

gchar * user_get_username (user_t *user);

gchar * user_get_language (user_t *user);

gchar * user_get_cookie (user_t *user);

credentials_t * credentials_new (user_t *user, const char *language,
                                 const char *client_address);

void credentials_free (credentials_t *creds);

int logout (credentials_t *);

void init_users ();

#endif /* _GSAD_USER_H_ */
