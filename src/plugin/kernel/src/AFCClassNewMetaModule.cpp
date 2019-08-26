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

#include "kernel/include/AFCClassNewMetaModule.hpp"
#include "base/AFXml.hpp"
#include "base/AFPluginManager.hpp"

namespace ark {

bool AFCClassNewMetaModule::Init()
{
    m_pLogModule = FindModule<AFILogModule>();

    m_pClassMetaManager = ARK_NEW AFClassMetaManager;

    ARK_ASSERT_RET_VAL(Load(), false);

    return true;
}

bool AFCClassNewMetaModule::Shut()
{
    ARK_DELETE(m_pClassMetaManager);

    return true;
}

bool AFCClassNewMetaModule::Load()
{
    if (!LoadConfig())
    {
        return false;
    }

    if (!LoadEntity())
    {
        return false;
    }

    return true;
}

bool AFCClassNewMetaModule::LoadConfig()
{
    std::string file_path = GetPluginManager()->GetResPath() + config_class_file;

    ARK_LOG_INFO("Load config class files: {}", file_path);

    AFXml xml_doc(file_path);
    auto root_node = xml_doc.GetRootNode();
    if (!root_node.IsValid())
    {
        ARK_ASSERT_NO_EFFECT(0);
        return false;
    }

    for (auto meta_node = root_node.FindNode("config"); meta_node.IsValid(); meta_node.NextNode())
    {
        std::string meta_name = meta_node.GetString("id");
        std::string schema_path = meta_node.GetString("meta");
        std::string res_path = meta_node.GetString("res");

        auto pClassMeta = m_pClassMetaManager->CreateMeta(meta_name);
        ARK_ASSERT_RET_VAL(pClassMeta != nullptr, false);

        pClassMeta->SetResPath(res_path);

        if (!LoadConfigMeta(schema_path, pClassMeta))
        {
            return false;
        }
    }

    return true;
}

bool AFCClassNewMetaModule::LoadConfigMeta(const std::string& schema_path, ARK_SHARE_PTR<AFClassMeta> pClassMeta)
{
    std::string file_path = GetPluginManager()->GetResPath() + schema_path;

    ARK_LOG_INFO("Load meta files: {}", schema_path);

    AFXml xml_doc(file_path);
    auto root_node = xml_doc.GetRootNode();
    if (!root_node.IsValid())
    {
        ARK_ASSERT_NO_EFFECT(0);
        return false;
    }

    for (auto meta_node = root_node.FindNode("meta"); meta_node.IsValid(); meta_node.NextNode())
    {
        std::string name = meta_node.GetString("field");
        std::string type_name = meta_node.GetString("type");

        ArkDataType data_type = ConvertDataType(type_name);
        ARK_ASSERT_RET_VAL(data_type != ArkDataType::DT_EMPTY, false);

        auto pDataMeta = pClassMeta->CreateDataMeta(name);
        ARK_ASSERT_RET_VAL(pDataMeta != nullptr, false);

        pDataMeta->SetType(data_type);
    }

    return true;
}

bool AFCClassNewMetaModule::LoadEntity()
{
    std::string file_path = GetPluginManager()->GetResPath() + entity_class_file;

    ARK_LOG_INFO("Load entity class files: {}", file_path);

    AFXml xml_doc(file_path);
    auto root_node = xml_doc.GetRootNode();
    if (!root_node.IsValid())
    {
        ARK_ASSERT_NO_EFFECT(0);
        return false;
    }

    for (auto meta_node = root_node.FindNode("meta"); meta_node.IsValid(); meta_node.NextNode())
    {
        std::string class_name = meta_node.GetString("class");
        std::string data_name = meta_node.GetString("field_name");
        std::string type_name = meta_node.GetString("type");
        std::string type_class = meta_node.GetString("type_class");

        // data mask
        uint32_t mask_save = meta_node.GetUint32("save");
        uint32_t mask_sync_self = meta_node.GetUint32("sync_self");
        // uint32_t mask_real_time = meta_node.GetUint32("real_time");

        // class meta
        auto pClassMeta = m_pClassMetaManager->CreateMeta(class_name);
        ARK_ASSERT_RET_VAL(pClassMeta != nullptr, false);

        if (type_name == "class")
        {
            // parent or child class type
            pClassMeta->AddClass(type_class);
            m_pClassMetaManager->AddTypeClassMeta(type_class);
            continue;
        }
        else if (type_name == "container")
        {
            pClassMeta->AddContainer(data_name, type_class);
            continue;
        }

        // convert type
        ArkDataType data_type = ConvertDataType(type_name);
        ARK_ASSERT_RET_VAL(data_type != ArkDataType::DT_EMPTY, false);

        if (ArkDataType::DT_TABLE == data_type)
        {
            // create record meta
            auto pRecordMeta = pClassMeta->CreateRecordMeta(data_name);
            ARK_ASSERT_RET_VAL(pRecordMeta != nullptr, false);

            pRecordMeta->SetTypeName(type_class);

            if (mask_save > 0)
            {
                pRecordMeta->SetFeatureType(ArkTableNodeFeature::PF_SAVE);
            }

            if (mask_sync_self > 0)
            {
                pRecordMeta->SetFeatureType(ArkTableNodeFeature::PF_PRIVATE);
            }

            m_pClassMetaManager->AddTypeClassMeta(type_class);
        }
        else
        {
            // default create data meta
            auto pDataMeta = pClassMeta->CreateDataMeta(data_name);
            ARK_ASSERT_RET_VAL(pDataMeta != nullptr, false);

            if (mask_save > 0)
            {
                pDataMeta->SetMask(ArkDataMaskType::DMT_SAVE_DB);
            }

            if (mask_sync_self > 0)
            {
                pDataMeta->SetMask(ArkDataMaskType::DMT_SYNC_SELF);
            }

            pDataMeta->SetType(data_type);
        }
    }

    // exact record meta and object meta of a class meta after all loaded
    ARK_ASSERT_RET_VAL(m_pClassMetaManager->AfterLoaded(), false);

    return true;
}

ArkDataType AFCClassNewMetaModule::ConvertDataType(const std::string& type_name)
{
    ArkDataType data_type = ArkDataType::DT_EMPTY;
    if (type_name == "bool")
    {
        data_type = ArkDataType::DT_BOOLEAN;
    }
    else if (type_name == "int")
    {
        data_type = ArkDataType::DT_INT32;
    }
    else if (type_name == "int32")
    {
        data_type = ArkDataType::DT_INT32;
    }
    else if (type_name == "int64")
    {
        data_type = ArkDataType::DT_INT64;
    }
    else if (type_name == "uint32")
    {
        data_type = ArkDataType::DT_UINT32;
    }
    else if (type_name == "uint64")
    {
        data_type = ArkDataType::DT_UINT64;
    }
    else if (type_name == "float")
    {
        data_type = ArkDataType::DT_FLOAT;
    }
    else if (type_name == "double")
    {
        data_type = ArkDataType::DT_DOUBLE;
    }
    else if (type_name == "string")
    {
        data_type = ArkDataType::DT_STRING;
    }
    else if (type_name == "table")
    {
        data_type = ArkDataType::DT_TABLE;
    }
    else if (type_name == "object")
    {
        data_type = ArkDataType::DT_OBJECT;
    }

    return data_type;
}

bool AFCClassNewMetaModule::AddClassCallBack(const std::string& class_name, CLASS_EVENT_FUNCTOR&& cb)
{
    auto pClassMeta = m_pClassMetaManager->FindMeta(class_name);
    ARK_ASSERT_RET_VAL(pClassMeta != nullptr, false);

    auto pCallBack = pClassMeta->GetClassCallBackManager();
    ARK_ASSERT_RET_VAL(pCallBack != nullptr, false);

    return pCallBack->AddClassCallBack(std::forward<CLASS_EVENT_FUNCTOR>(cb));
}

bool AFCClassNewMetaModule::DoEvent(
    const AFGUID& id, const std::string& class_name, const ArkEntityEvent class_event, const AFIDataList& args)
{
    auto pClassMeta = m_pClassMetaManager->FindMeta(class_name);
    ARK_ASSERT_RET_VAL(pClassMeta != nullptr, false);

    auto pCallBack = pClassMeta->GetClassCallBackManager();
    ARK_ASSERT_RET_VAL(pCallBack != nullptr, false);

    return pCallBack->DoEvent(id, class_name, class_event, args);
}

ARK_SHARE_PTR<AFClassMeta> AFCClassNewMetaModule::FindMeta(const std::string& class_name)
{
    return m_pClassMetaManager->FindMeta(class_name);
}

const AFClassMetaManager::ClassMetaList& AFCClassNewMetaModule::GetMetaList() const
{
    return m_pClassMetaManager->GetMetaList();
}

} // namespace ark
