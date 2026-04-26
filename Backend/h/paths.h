#pragma once

#include <filesystem>

const std::filesystem::path base = std::filesystem::path(__FILE__).parent_path().parent_path();

const auto VOICES_SAMPLES_PATH = base / "local" / "samples";
const auto INSTRUMENT_PATH = base / "local" / "samples" / "instrument";
const auto SAMPLE_OUTPUT_PATH = base / "local" / "out" / "sample.wav";
const auto SYNTH_OUTPUT_PATH = base / "local" / "out" / "synth.wav";
const auto MODEL_OUTPUT_PATH = base / "local" / "out" / "model.wav";
const auto TEST_PARAMS_PATH = base / "local" / "test" / "temp.json";
const auto TEST_OUTPUT_PATH_COMP = base / "local" / "test" / "comp.wav";
const auto TEST_OUTPUT_PATH_REF = base / "local" / "test" / "ref.wav";
const std::string SAMPLE_FORMAT = ".wav";

const std::string RELEASE_POSTFIX = "_release";
const std::string ATTACK_POSTFIX = "_attack";