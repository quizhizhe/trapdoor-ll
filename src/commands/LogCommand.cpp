//
// Created by xhy on 2022/5/17.
//
#include <mc/Level.hpp>
#include <mc/LevelSeed64.hpp>
#include <unordered_map>

#include "CommandHelper.h"
#include "DynamicCommandAPI.h"
#include "MCTick.h"
#include "Msg.h"
#include "SysInfoHelper.h"
namespace trapdoor {
    namespace {

        ActionResult getLevelSeed() {
            auto seed = Global<Level>->getLevelSeed64().to32BitRandomSeed();
            trapdoor::TextBuilder builder;
            builder.text("Level seed is ").num(seed);
            return {builder.get(), true};
        }
        ActionResult getPlayerEnchantSeed(Player *p) {
            if (!p) return ErrorPlayerNeed();
            auto seed = p->getEnchantmentSeed();
            trapdoor::TextBuilder builder;
            builder.textF("Player %s's enchant seed is ", p->getRealName().c_str()).num(seed);
            return {builder.get(), true};
        }
    }  // namespace

    void setup_logCommand(int level) {
        using ParamType = DynamicCommand::ParameterType;
        // create a dynamic command
        auto command = CREATE_CMD(log, level);

        auto &optMain = command->setEnum("main", {"mspt", "os"});
        auto &optSeed = command->setEnum("seeds", {"levelseed", "enchantseed"});

        auto &optPendingTick = command->setEnum("pts", {"pt"});

        command->mandatory("log", ParamType::Enum, optMain,
                           CommandParameterOption::EnumAutocompleteExpansion);
        command->mandatory("log", ParamType::Enum, optSeed,
                           CommandParameterOption::EnumAutocompleteExpansion);

        command->mandatory("log", ParamType::Enum, optPendingTick,
                           CommandParameterOption::EnumAutocompleteExpansion);

        command->optional("blockPos", ParamType::BlockPos);

        command->addOverload({optMain});
        command->addOverload({optSeed});
        command->addOverload({optPendingTick, "blockPos"});

        auto cb = [](DynamicCommand const &command, CommandOrigin const &origin,
                     CommandOutput &output,
                     std::unordered_map<std::string, DynamicCommand::Result> &results) {
            auto pos = results["blockPos"].isSet ? results["p1"].get<BlockPos>() : BlockPos::MAX;
            switch (do_hash(results["log"].getRaw<std::string>().c_str())) {
                case do_hash("mspt"):
                    trapdoor::printMSPT().sendTo(output);
                    break;
                case do_hash("os"):
                    trapdoor::printSysInfo().sendTo(output);
                    break;
                case do_hash("levelseed"):
                    getLevelSeed().sendTo(output);
                    break;
                case do_hash("enchantseed"):
                    getPlayerEnchantSeed(origin.getPlayer()).sendTo(output);
                    break;
                case do_hash("pt"):
                    trapdoor::getPendingTickInfo(origin.getPlayer(), pos).sendTo(output);
                    break;
                default:
                    break;
            }
        };
        command->setCallback(cb);

        DynamicCommand::setup(std::move(command));
    }
}  // namespace trapdoor