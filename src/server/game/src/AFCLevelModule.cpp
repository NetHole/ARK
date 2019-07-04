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

#include "interface/AFIPluginManager.h"
#include "game/include/AFCLevelModule.h"

namespace ark
{

    bool AFCLevelModule::Init()
    {
        m_pKernelModule = pPluginManager->FindModule<AFIKernelModule>();
        m_pLogModule = pPluginManager->FindModule<AFILogModule>();
        m_pPropertyConfigModule = pPluginManager->FindModule<AFIPropertyConfigModule>();
        m_pConfigModule = pPluginManager->FindModule<AFIConfigModule>();

        return true;
    }

    int AFCLevelModule::AddExp(const AFGUID& self, const int nExp)
    {
        int eJobType = m_pKernelModule->GetNodeInt(self, AFEntityMetaPlayer::career());
        int nCurExp = m_pKernelModule->GetNodeInt(self, AFEntityMetaPlayer::exp());
        int nLevel = m_pKernelModule->GetNodeInt(self, AFEntityMetaPlayer::level());
        int nMaxExp = m_pPropertyConfigModule->CalculateBaseValue(eJobType, nLevel, AFEntityMetaPlayer::exp());//xx

        nCurExp += nExp;

        int nRemainExp = nCurExp - nMaxExp;

        while (nRemainExp >= 0)
        {
            //升级
            nLevel++;
            //防止越级BUG
            m_pKernelModule->SetNodeInt(self, AFEntityMetaPlayer::level(), nLevel);

            nCurExp = nRemainExp;

            nMaxExp = m_pPropertyConfigModule->CalculateBaseValue(eJobType, nLevel, AFEntityMetaPlayer::exp());//xx

            if (nMaxExp <= 0)
            {
                break;
            }

            nRemainExp -= nMaxExp;
        }

        m_pKernelModule->SetNodeInt(self, AFEntityMetaPlayer::exp(), nCurExp);

        return 0;
    }

}