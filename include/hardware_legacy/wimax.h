/*
 * Copyright 2011, The CyanogenMod Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if defined(_WIN32) || defined(_WIN64)
#ifdef WIMAX_EXPORTS
#define WIMAX_API __declspec(dllexport)
#else
#define WIMAX_API __declspec(dllimport)
#endif
#else
#ifdef _DYLIB_
#define WIMAX_API __attribute__((visibility("default")))
#else
#define WIMAX_API
#endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Stop the Wimax DHCP.
 *
 * @return 0 on success, < 0 on failure.
 */
int stopDhcpWimax();
	
/**
 * Load the Wimax driver.
 *
 * @return 0 on success, < 0 on failure.
 */
int loadWimaxDriver();

/**
 * Unload the Wi-Fi driver.
 *
 * @return 0 on success, < 0 on failure.
 */
int unloadWimaxDriver();

/**
 * Start supplicant.
 *
 * @return 0 on success, < 0 on failure.
 */
int startWimaxDaemon();

/**
 * Stop supplicant.
 *
 * @return 0 on success, < 0 on failure.
 */
int stopWimaxDaemon();

int getWimaxProp();

int setWimaxProp();

int startDhcpWimaxDaemon();

int dhcpRelease();

int wimaxDhcpRenew();

int addRouteToGateway();

int terminateProcess(char *pid);

int doWimaxDhcpRequest(int *ipaddr, int *gateway, int *mask,
                    int *dns1, int *dns2, int *server, int *lease);

const char *getWimaxDhcpError();

int getBlockSize();

#ifdef __cplusplus
}
#endif
