/*
 * This source file is part of ARK
 * For the latest info, see https://github.com/ArkNX
 *
 * Copyright (c) 2013-2019 ArkNX authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"),
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
 *
 */

#include "kernel/include/AFKernelPlugin.hpp"
#include "kernel/include/AFCKernelModule.hpp"
#include "kernel/include/AFCMetaClassModule.hpp"
#include "kernel/include/AFCConfigModule.hpp"
#include "kernel/include/AFCMapModule.hpp"

namespace ark {

ARK_PLUGIN_DECLARE(AFKernelPlugin)

int AFKernelPlugin::GetPluginVersion()
{
    return 0;
}

const std::string AFKernelPlugin::GetPluginName()
{
    return GET_CLASS_NAME(AFKernelPlugin);
}

void AFKernelPlugin::Install()
{
    RegisterModule<AFIMetaClassModule, AFCMetaClassModule>();
    RegisterModule<AFIConfigModule, AFCConfigModule>();
    RegisterModule<AFIKernelModule, AFCKernelModule>();
    RegisterModule<AFIMapModule, AFCMapModule>();
}

void AFKernelPlugin::Uninstall()
{
    DeregisterModule<AFIMapModule, AFCMapModule>();
    DeregisterModule<AFIKernelModule, AFCKernelModule>();
    DeregisterModule<AFIConfigModule, AFCConfigModule>();
    DeregisterModule<AFIMetaClassModule, AFCMetaClassModule>();
}

} // namespace ark
