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

#include "gsad_user.h"
#include "gsad_base.h" /* for set_language_code */
#include "gsad_settings.h"
#include "gsad_gmp_auth.h"
#include "utils.h"

#include <assert.h> /* for asset */
#include <string.h> /* for strcmp */

#include <gvm/util/uuidutils.h> /* for gvm_uuid_make */


/**
 * @brief User session data.
 */
GPtrArray *users = NULL;

/**
 * @brief Mutex to prevent concurrent access to user information.
 */
static GMutex *mutex = NULL;

/**
 * @brief User information structure, for sessions.
 */
struct user
{
  gchar *cookie;        ///< Cookie token.
  gchar *token;         ///< Request session token.
  gchar *username;      ///< Login name.
  gchar *password;      ///< Password.
  gchar *role;          ///< Role.
  gchar *timezone;      ///< Timezone.
  gchar *severity;      ///< Severity class.
  gchar *capabilities;  ///< Capabilities.
  gchar *language;      ///< User Interface Language, in short form like "en".
  gchar *pw_warning;    ///< Password policy warning.
  gchar *address;       ///< Client's IP address.
  time_t time;          ///< Login time.
  gboolean guest;       ///< Whether the user is a guest.
};

user_t *
user_new()
{
  user_t *user = g_malloc0 (sizeof (user_t));
  return user;
}

void
user_free (user_t *user)
{
  g_free(user->cookie);
  g_free(user->token);
  g_free(user->username);
  g_free(user->password);
  g_free(user->role);
  g_free(user->timezone);
  g_free(user->severity);
  g_free(user->capabilities);
  g_free(user->language);
  g_free(user->pw_warning);
  g_free(user->address);
  g_free(user);
}

user_t *
user_copy (user_t *user)
{
  user_t *copy = user_new();

  copy->cookie = g_strdup(user->cookie);
  copy->token = g_strdup(user->token);
  copy->username = g_strdup(user->username);
  copy->password = g_strdup(user->password);
  copy->role = g_strdup(user->role);
  copy->timezone = g_strdup(user->timezone);
  copy->severity = g_strdup(user->severity);
  copy->capabilities = g_strdup(user->capabilities);
  copy->language = g_strdup(user->language);
  copy->pw_warning = g_strdup(user->pw_warning);
  copy->address = g_strdup(user->address);
  copy->time = user->time;
  copy->guest = user->guest;

  return copy;
}

gboolean
user_session_expired (user_t *user)
{
  return (time (NULL) - user->time) > (get_session_timeout () * 60);
}

gchar *
user_get_username (user_t *user)
{
  return user->username;
}

gchar *
user_get_language (user_t *user)
{
  return user->language;
}

gchar *
user_get_cookie (user_t *user)
{
  return user->cookie;
}

void
user_renew_session (user_t *user)
{
  user->time = time (NULL);
}

user_t *
get_user_by_token (const gchar *token)
{
  int index;
  user_t * user = NULL;

  g_mutex_lock (mutex);

  for (index = 0; index < users->len; index++)
    {
      user_t *item;
      item = (user_t*) g_ptr_array_index (users, index);
      if (str_equal (item->token, token))
        {
          user = user_copy(item);
          break;
        }
    }

  g_mutex_unlock (mutex);

  return user;
}

user_t *
get_user_by_username (const gchar *username)
{
  int index;
  user_t * user = NULL;

  g_mutex_lock (mutex);

  for (index = 0; index < users->len; index++)
    {
      user_t *item;
      item = (user_t*) g_ptr_array_index (users, index);
      if (str_equal (item->username, username))
        {
          user = user_copy(item);
          break;
        }
    }

  g_mutex_unlock (mutex);

  return user;
}

/**
 * @brief Add a user.
 *
 * Creates and initializes a user object with given parameters
 *
 * It's up to the caller to free the returned user.
 *
 * @param[in]  username      Name of user.
 * @param[in]  password      Password for user.
 * @param[in]  timezone      Timezone of user.
 * @param[in]  severity      Severity class setting of user.
 * @param[in]  role          Role of user.
 * @param[in]  capabilities  Capabilities of manager.
 * @param[in]  language      User Interface Language (language name or code)
 * @param[in]  pw_warning    Password policy warning.
 * @param[in]  address       Client's IP address.
 *
 * @return Added user.
 */
user_t *
user_add (const gchar *username, const gchar *password, const gchar *timezone,
          const gchar *severity, const gchar *role, const gchar *capabilities,
          const gchar *language, const gchar *pw_warning, const char *address)
{
  const gchar * guest_username = get_guest_username ();

  user_t *user = get_user_by_username (username);

  if (user && user_session_expired (user))
    {
      user_remove (user);
    }

  user = user_new();
  user_renew_session (user);
  user->cookie = gvm_uuid_make ();
  user->token = gvm_uuid_make ();
  user->username = g_strdup (username);
  user->password = g_strdup (password);
  user->role = g_strdup (role);
  user->timezone = g_strdup (timezone);
  user->severity = g_strdup (severity);
  user->capabilities = g_strdup (capabilities);
  user->pw_warning = pw_warning ? g_strdup (pw_warning) : NULL;

  set_language_code (&user->language, language);

  if (guest_username)
    user->guest = str_equal (username, guest_username) ? 1 : 0;
  else
    user->guest = 0;

  user->address = g_strdup (address);

  g_mutex_lock (mutex);

  g_ptr_array_add (users, (gpointer) user_copy(user));

  g_mutex_unlock (mutex);

  return user;
}

/**
 * @brief Find a user, given a token and cookie.
 *
 * If a user is returned, the session of the user is renewed and it's up to the
 * caller to free the user.
 *
 * @param[in]   cookie       Token in cookie.
 * @param[in]   token        Token request parameter.
 * @param[in]   address      Client's IP address.
 * @param[out]  user_return  User.
 *
 * @return 0 ok (user in user_return),
 *         1 bad token,
 *         2 expired token,
 *         3 bad/missing cookie,
 *         4 bad/missing token,
 *         5 guest login failed,
 *         6 GMP down for guest login,
 *         7 IP address mismatch,
 *        -1 error during guest login.
 */
int
user_find (const gchar *cookie, const gchar *token, const char *address,
           user_t **user_return)
{
  user_t *user = NULL;
  int index;
  const gchar * guest_username = get_guest_username ();
  const gchar * guest_password = get_guest_password ();

  if (token == NULL)
    return USER_BAD_MISSING_TOKEN;

  if (guest_username && token && str_equal (token, "guest"))
    {
      int ret;
      gchar *timezone, *role, *capabilities, *severity, *language;
      gchar *pw_warning;

      if (cookie)
        {
          /* Look for an existing guest user from the same browser (that is,
           * with the same cookie). */

          g_mutex_lock (mutex);

          for (index = 0; index < users->len; index++)
            {
              user_t *item;
              item = (user_t*) g_ptr_array_index (users, index);
              if (item->guest && str_equal (item->cookie, cookie))
                {
                  user = user_copy (item);
                  break;
                }
            }

          g_mutex_unlock (mutex);

          if (user)
            {
              *user_return = user;
              user_renew_session (user);
              return USER_OK;
            }
        }

      /* Log in as guest. */

      ret = authenticate_gmp (guest_username,
                              guest_password,
                              &role,
                              &timezone,
                              &severity,
                              &capabilities,
                              &language,
                              &pw_warning);
      if (ret == 1)
        return USER_GUEST_LOGIN_FAILED;
      else if (ret == 2)
        return USER_GMP_DOWN;
      else if (ret == -1)
        return USER_GUEST_LOGIN_ERROR;
      else
        {
          user_t *user;
          user = user_add (guest_username, guest_password, timezone, severity,
                           role, capabilities, language, pw_warning, address);
          *user_return = user;
          g_free (timezone);
          g_free (severity);
          g_free (capabilities);
          g_free (language);
          g_free (role);
          g_free (pw_warning);
          return USER_OK;
        }
    }

  user = get_user_by_token (token);

  if (user)
    {
      if (user_session_expired (user))
        {
          user_free (user);
          return USER_EXPIRED_TOKEN;
        }

      else if ((cookie == NULL) || !str_equal (user->cookie, cookie))
        {
          user_free (user);
          return USER_BAD_MISSING_COOKIE;
        }

      /* Verify that the user address matches the client's address. */
      else if (!str_equal (address, user->address))
        {
          user_free (user);
          return USER_IP_ADDRESS_MISSMATCH;
        }
      else
        {
          *user_return = user;
          // renew session time
          user_renew_session (user);

          // mutex will be unlocked with user_release
          return USER_OK;
        }
    }

    /* should it be really USER_EXPIRED_TOKEN?
    * No user has been found therefore the token couldn't even expire */
  return USER_EXPIRED_TOKEN;
}

/**
 * @brief Set timezone of user.
 *
 * @param[in]   token     User token.
 * @param[in]   timezone  Timezone.
 *
 * @return 0 ok, 1 failed to find user.
 */
int
user_set_timezone (const gchar *token, const gchar *timezone)
{
  int ret = 1;

  user_t * user = get_user_by_token (token);

  if (user)
    {
      g_free (user->timezone);

      user->timezone = g_strdup (timezone);

      ret = 0;
    }

  return ret;
}

/**
 * @brief Set password of user.
 *
 * @param[in]   token     User token.
 * @param[in]   password  Password.
 *
 * @return 0 ok, 1 failed to find user.
 */
int
user_set_password (const gchar *token, const gchar *password)
{
  int ret = 1;

  user_t *user = get_user_by_token (token);

  if (user)
    {
      g_free (user->password);
      g_free (user->pw_warning);

      user->password = g_strdup (password);
      user->pw_warning = NULL;

      ret = 0;
    }

  return ret;
}

/**
 * @brief Set severity class of user.
 *
 * @param[in]   token     User token.
 * @param[in]   severity  Severity class.
 *
 * @return 0 ok, 1 failed to find user.
 */
int
user_set_severity (const gchar *token, const gchar *severity)
{
  int ret = 1;

  user_t *user = get_user_by_token (token);

  if (user)
    {
      g_free (user->severity);

      user->severity = g_strdup (severity);

      ret = 0;
    }

  return ret;
}

/**
 * @brief Set language of user.
 *
 * @param[in]   token     User token.
 * @param[in]   language  Language.
 *
 * @return 0 ok, 1 failed to find user.
 */
int
user_set_language (const gchar *token, const gchar *language)
{
  int ret = 1;

  user_t *user = get_user_by_token (token);

  if (user)
    {
      g_free (user->language);

      set_language_code (&user->language, language);

      ret = 0;
    }

  return ret;
}

/**
 * @brief Logs out all sessions of a given user, except the current one.
 *
 * @param[in]   username        User name.
 * @param[in]   credentials     Current user's credentials.
 *
 * @return 0 ok, -1 error.
 */
int
user_logout_all_sessions (const gchar *username, credentials_t *credentials)
{
  int index;

  g_mutex_lock (mutex);

  for (index = 0; index < users->len; index++)
    {
      user_t *item;
      item = (user_t*) g_ptr_array_index (users, index);

      if (str_equal (item->username, username)
          && !str_equal (item->token, credentials->token))
        {
          g_debug ("%s: logging out user '%s', token '%s'",
                   __FUNCTION__, item->username, item->token);
          g_ptr_array_remove (users, (gpointer) item);

          user_free (item);

          index --;
        }
    }

  g_mutex_unlock (mutex);

  return 0;
}

/**
 * @brief Remove a user from the session "database", freeing the user_t too.
 *
 * @param[in]  user  User.
 */
void
user_remove (user_t *user)
{
  g_mutex_lock (mutex);
  g_ptr_array_remove (users, (gpointer) user);
  g_mutex_unlock (mutex);

  user_free(user);
}

credentials_t *
credentials_new (user_t *user, const char *language, const char *client_address)
{
  credentials_t *credentials;

  assert (user->username);
  assert (user->password);
  assert (user->role);
  assert (user->timezone);
  assert (user->capabilities);
  assert (user->token);
  credentials = g_malloc0 (sizeof (credentials_t));
  credentials->username = g_strdup (user->username);
  credentials->password = g_strdup (user->password);
  credentials->role = g_strdup (user->role);
  credentials->timezone = g_strdup (user->timezone);
  credentials->severity = g_strdup (user->severity);
  credentials->capabilities = g_strdup (user->capabilities);
  credentials->token = g_strdup (user->token);
  credentials->pw_warning = user->pw_warning ? g_strdup (user->pw_warning)
                                             : NULL;
  credentials->language = g_strdup (language);
  credentials->client_address = g_strdup (client_address);
  credentials->guest = user->guest;
  credentials->sid = g_strdup (user->cookie);

  return credentials;
}

void
credentials_free (credentials_t *creds)
{
  if (!creds)
    return;

  g_free (creds->username);
  g_free (creds->password);
  g_free (creds->role);
  g_free (creds->timezone);
  g_free (creds->token);
  g_free (creds->caller);
  g_free (creds->current_page);
  g_free (creds->capabilities);
  g_free (creds->language);
  g_free (creds->severity);
  g_free (creds->pw_warning);
  g_free (creds->client_address);
  g_free (creds->sid);
  /* params and chart_prefs are not duplicated. */
  g_free (creds);
}

/**
 * @brief Removes the users token
 *
 * @param[in]  credentials  Use credentials.
 *
 * @return 0 success, -1 error.
 */
int
logout (credentials_t *credentials)
{
  if (credentials->token == NULL)
    return 0;

  user_t * user = get_user_by_token (credentials->token);

  if (user)
    {
      user_remove (user);
      return 0;
    }

  return -1;
}

void
init_users ()
{
  mutex = g_malloc (sizeof (GMutex));
  g_mutex_init (mutex);
  users = g_ptr_array_new ();
}

