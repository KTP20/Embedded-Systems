/*
 * Google Cloud Certificates
 * Copyright (c) 2019-2020, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef AZURE_CLOUD_CREDENTIALS_H
#define AZURE_CLOUD_CREDENTIALS_H

namespace azure_cloud {
namespace credentials {
/*
 * Primary Connecion String
 */

// Use https://dpsgen.z8.web.core.windows.net/ to obtain YOUR connection string
// This one will not work, but I guess you knew that ;)
//                                 HostName=iotc-fbab197f-edaf-440b-a90a-fff2e43d3ec1.azure-devices.netDeviceId=fvgk75mov7;SharedAccessKey=mcmDDGOzTkINwERiEUOgWeMQjD7gT1rSVu+WeGTYK6o=
char iothub_connection_string[] = "HostName=iotc-fbab197f-edaf-440b-a90a-fff2e43d3ec1.azure-devices.net;DeviceId=fvgk75mov7;SharedAccessKey=mcmDDGOzTkINwERiEUOgWeMQjD7gT1rSVu+WeGTYK6o=";
}
}
#endif
