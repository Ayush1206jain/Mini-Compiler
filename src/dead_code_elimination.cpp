#include "dead_code_elimination.hpp"
#include <algorithm>

DeadCodeElimination::DeadCodeElimination(int maxPasses)
    : maxPasses_(maxPasses) {}

bool DeadCodeElimination::hasSideEffects(const IRInstruction& inst) {
    // PRINT produces observable output
    if (inst.opcode == IROpcode::PRINT) {
        return true;
    }
    // Control flow instructions alter execution path
    if (inst.opcode == IROpcode::LABEL ||
        inst.opcode == IROpcode::JUMP ||
        inst.opcode == IROpcode::JUMP_IF_FALSE) {
        return true;
    }
    return false;
}

bool DeadCodeElimination::isPotentiallyTrapping(const IRInstruction& inst) {
    // Division can trap with division-by-zero at runtime under language semantics.
    // Preserving potentially trapping operations ensures runtime errors remain observable.
    if (inst.opcode == IROpcode::DIV) {
        return true;
    }
    return false;
}

bool DeadCodeElimination::isPure(const IRInstruction& inst) {
    if (hasSideEffects(inst) || isPotentiallyTrapping(inst)) {
        return false;
    }
    switch (inst.opcode) {
        case IROpcode::LOAD_CONST:
        case IROpcode::MOVE:
        case IROpcode::ADD:
        case IROpcode::SUB:
        case IROpcode::MUL:
        case IROpcode::CMP_EQ:
        case IROpcode::CMP_NE:
        case IROpcode::CMP_LT:
        case IROpcode::CMP_LE:
        case IROpcode::CMP_GT:
        case IROpcode::CMP_GE:
            return true;
        default:
            return false;
    }
}

std::vector<std::string> DeadCodeElimination::getOperandsUsed(const IRInstruction& inst) {
    std::vector<std::string> uses;
    switch (inst.opcode) {
        case IROpcode::LOAD_CONST:
            // No variable operands used
            break;
        case IROpcode::MOVE:
            if (!inst.arg1.empty()) uses.push_back(inst.arg1);
            break;
        case IROpcode::ADD:
        case IROpcode::SUB:
        case IROpcode::MUL:
        case IROpcode::DIV:
        case IROpcode::CMP_EQ:
        case IROpcode::CMP_NE:
        case IROpcode::CMP_LT:
        case IROpcode::CMP_LE:
        case IROpcode::CMP_GT:
        case IROpcode::CMP_GE:
            if (!inst.arg1.empty()) uses.push_back(inst.arg1);
            if (!inst.arg2.empty()) uses.push_back(inst.arg2);
            break;
        case IROpcode::PRINT:
            if (!inst.arg1.empty()) uses.push_back(inst.arg1);
            break;
        case IROpcode::JUMP_IF_FALSE:
            // arg1 is the condition variable
            if (!inst.arg1.empty()) uses.push_back(inst.arg1);
            break;
        case IROpcode::LABEL:
        case IROpcode::JUMP:
            // No data operands used
            break;
    }
    return uses;
}

std::string DeadCodeElimination::getDestinationDefined(const IRInstruction& inst) {
    switch (inst.opcode) {
        case IROpcode::LOAD_CONST:
        case IROpcode::MOVE:
        case IROpcode::ADD:
        case IROpcode::SUB:
        case IROpcode::MUL:
        case IROpcode::DIV:
        case IROpcode::CMP_EQ:
        case IROpcode::CMP_NE:
        case IROpcode::CMP_LT:
        case IROpcode::CMP_LE:
        case IROpcode::CMP_GT:
        case IROpcode::CMP_GE:
            return inst.dest;
        case IROpcode::PRINT:
        case IROpcode::LABEL:
        case IROpcode::JUMP:
        case IROpcode::JUMP_IF_FALSE:
            return "";
    }
    return "";
}

namespace {

struct BasicBlock {
    size_t id = 0;
    size_t start = 0; // Inclusive instruction index
    size_t end = 0;   // Exclusive instruction index
    std::vector<size_t> successors;
    std::unordered_set<std::string> gen;
    std::unordered_set<std::string> kill;
    std::unordered_set<std::string> in;
    std::unordered_set<std::string> out;
};

} // namespace

bool DeadCodeElimination::runSinglePass(IRProgram& program) {
    const size_t n = program.instructions.size();
    if (n == 0) return false;

    // Step 1: Identify Basic Block Leaders
    std::vector<bool> isLeader(n, false);
    isLeader[0] = true;

    for (size_t i = 0; i < n; ++i) {
        const auto& inst = program.instructions[i];
        if (inst.opcode == IROpcode::LABEL) {
            isLeader[i] = true;
        } else if (inst.opcode == IROpcode::JUMP || inst.opcode == IROpcode::JUMP_IF_FALSE) {
            if (i + 1 < n) {
                isLeader[i + 1] = true;
            }
        }
    }

    // Step 2: Form Basic Blocks
    std::vector<BasicBlock> blocks;
    std::vector<size_t> instToBlock(n, 0);
    std::unordered_map<std::string, size_t> labelToBlock;

    size_t currentStart = 0;
    for (size_t i = 1; i < n; ++i) {
        if (isLeader[i]) {
            BasicBlock block;
            block.id = blocks.size();
            block.start = currentStart;
            block.end = i;
            for (size_t j = currentStart; j < i; ++j) {
                instToBlock[j] = block.id;
            }
            // Check if first instruction is a label
            if (program.instructions[currentStart].opcode == IROpcode::LABEL) {
                labelToBlock[program.instructions[currentStart].dest] = block.id;
            }
            blocks.push_back(block);
            currentStart = i;
        }
    }
    // Final block
    {
        BasicBlock block;
        block.id = blocks.size();
        block.start = currentStart;
        block.end = n;
        for (size_t j = currentStart; j < n; ++j) {
            instToBlock[j] = block.id;
        }
        if (program.instructions[currentStart].opcode == IROpcode::LABEL) {
            labelToBlock[program.instructions[currentStart].dest] = block.id;
        }
        blocks.push_back(block);
    }

    // Step 3: Connect Block Successors
    for (size_t b = 0; b < blocks.size(); ++b) {
        auto& block = blocks[b];
        if (block.start >= block.end) continue;
        const auto& lastInst = program.instructions[block.end - 1];

        if (lastInst.opcode == IROpcode::JUMP) {
            auto it = labelToBlock.find(lastInst.dest);
            if (it != labelToBlock.end()) {
                block.successors.push_back(it->second);
            }
        } else if (lastInst.opcode == IROpcode::JUMP_IF_FALSE) {
            // Branch target
            auto it = labelToBlock.find(lastInst.dest);
            if (it != labelToBlock.end()) {
                block.successors.push_back(it->second);
            }
            // Fall-through target
            if (b + 1 < blocks.size()) {
                block.successors.push_back(b + 1);
            }
        } else {
            // Sequential fall-through
            if (b + 1 < blocks.size()) {
                block.successors.push_back(b + 1);
            }
        }
    }

    // Step 4: Compute `gen` and `kill` sets for each basic block
    for (auto& block : blocks) {
        for (size_t i = block.start; i < block.end; ++i) {
            const auto& inst = program.instructions[i];
            auto uses = getOperandsUsed(inst);
            for (const auto& u : uses) {
                if (block.kill.find(u) == block.kill.end()) {
                    block.gen.insert(u);
                }
            }
            std::string def = getDestinationDefined(inst);
            if (!def.empty()) {
                block.kill.insert(def);
            }
        }
    }

    // Step 5: Iterative Backward Dataflow Liveness Analysis
    bool dataflowChanged = true;
    while (dataflowChanged) {
        dataflowChanged = false;

        // Iterate in reverse topological order (or reverse index order)
        for (int b = static_cast<int>(blocks.size()) - 1; b >= 0; --b) {
            auto& block = blocks[b];

            // Out[B] = Union_{S in Succ[B]} In[S]
            std::unordered_set<std::string> newOut;
            for (size_t succId : block.successors) {
                const auto& succIn = blocks[succId].in;
                newOut.insert(succIn.begin(), succIn.end());
            }

            // In[B] = gen[B] Union (Out[B] \ kill[B])
            std::unordered_set<std::string> newIn = block.gen;
            for (const auto& var : newOut) {
                if (block.kill.find(var) == block.kill.end()) {
                    newIn.insert(var);
                }
            }

            if (newIn != block.in || newOut != block.out) {
                block.in = std::move(newIn);
                block.out = std::move(newOut);
                dataflowChanged = true;
            }
        }
    }

    // Step 6: Identify and Mark Dead Instructions Within Each Block
    std::vector<bool> markedForRemoval(n, false);
    int passEliminated = 0;

    for (const auto& block : blocks) {
        std::unordered_set<std::string> activeLive = block.out;

        for (int i = static_cast<int>(block.end) - 1; i >= static_cast<int>(block.start); --i) {
            const auto& inst = program.instructions[i];

            if (isPure(inst)) {
                std::string dest = getDestinationDefined(inst);
                if (!dest.empty() && activeLive.find(dest) == activeLive.end()) {
                    // Instruction defines a value that is NOT live and has no side effects: DEAD!
                    markedForRemoval[i] = true;
                    passEliminated++;
                    // Do NOT add operands of removed instruction to activeLive
                    continue;
                }
            }

            // Instruction is preserved: update liveness backwards
            std::string dest = getDestinationDefined(inst);
            if (!dest.empty()) {
                activeLive.erase(dest);
            }
            auto uses = getOperandsUsed(inst);
            for (const auto& u : uses) {
                activeLive.insert(u);
            }
        }
    }

    if (passEliminated == 0) {
        return false;
    }

    // Step 7: Construct Compacted Program
    IRProgram compacted;
    compacted.instructions.reserve(n - passEliminated);
    for (size_t i = 0; i < n; ++i) {
        if (!markedForRemoval[i]) {
            compacted.addInstruction(program.instructions[i]);
        }
    }

    program = std::move(compacted);
    stats_.eliminatedInstructions += passEliminated;
    return true;
}

IRProgram DeadCodeElimination::eliminate(const IRProgram& program) {
    IRProgram result = program; // Work on a copy to preserve original IR
    stats_ = DCEStats();

    for (int pass = 0; pass < maxPasses_; ++pass) {
        stats_.passesRun++;
        bool changed = runSinglePass(result);
        if (!changed) {
            break; // Fixed-point convergence reached
        }
    }

    return result;
}
