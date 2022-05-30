
#include <MC/BlockInstance.hpp>
#include <MC/Dimension.hpp>
#include <string>

#include "CommandHelper.h"
#include "DynamicCommandAPI.h"
#include "SpawnHelper.h"

namespace tr {
    void SetupSpawnCommand() {
        using ParamType = DynamicCommand::ParameterType;
        // create a dynamic command
        auto command = DynamicCommand::createCommand(
            "spawn", "get spawn info", CommandPermissionLevel::GameMasters);

        // set enum就是给这个命令加一些enum值，不会产生任何意义
        auto &optCount = command->setEnum("CountCmd", {"count"});
        auto &prob = command->setEnum("probilityCmd", {"prob"});

        auto &optCountType =
            command->setEnum("counter options", {"chunk", "all", "density"});

        // mandatory/options就是给enum增加后置参数类型的,mandatory就是必填,optional是选填
        command->mandatory("spawn", ParamType::Enum, optCount,
                           CommandParameterOption::EnumAutocompleteExpansion);

        command->mandatory("spawn", ParamType::Enum, prob,
                           CommandParameterOption::EnumAutocompleteExpansion);

        command->mandatory("countType", ParamType::Enum, optCountType,
                           CommandParameterOption::EnumAutocompleteExpansion);

        command->optional("blockPos", ParamType::BlockPos);
        //添加子命令并进行类型绑定
        command->addOverload({prob, "blockPos"});

        // add
        // overload就是增加一些子命令，子命令需要Enum；并设定后面需要接什么类型的参数
        command->addOverload({optCount, "countType"});

        auto cb = [](DynamicCommand const &command, CommandOrigin const &origin,
                     CommandOutput &output,
                     std::unordered_map<std::string, DynamicCommand::Result>
                         &results) {
            auto countParam = std::string();
            switch (do_hash(results["spawn"].getRaw<std::string>().c_str())) {
                case do_hash("count"):
                    tr::CountActors(
                        reinterpret_cast<Player *>(origin.getPlayer()),
                        results["countType"].getRaw<std::string>())
                        .SendTo(output);
                    break;

                case do_hash("prob"):
                    if (results["blockPos"].isSet) {
                        tr::printSpawnProbability(
                            reinterpret_cast<Player *>(origin.getPlayer()),
                            results["blockPos"].get<BlockPos>())
                            .SendTo(output);
                    } else {
                        tr::printSpawnProbability(
                            reinterpret_cast<Player *>(origin.getPlayer()),
                            reinterpret_cast<Actor *>(origin.getPlayer())
                                ->getBlockFromViewVector()
                                .getPosition())
                            .SendTo(output);
                    }
            }
        };
        command->setCallback(cb);
        DynamicCommand::setup(std::move(command));
    }
}  // namespace tr