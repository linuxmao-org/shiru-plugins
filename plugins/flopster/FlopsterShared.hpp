#pragma once

__attribute__((unused)) static constexpr int STEP_SAMPLES_ALL = 80;

__attribute__((unused)) static constexpr int HEAD_BASE_NOTE = (12*4);
__attribute__((unused)) static constexpr int HEAD_BUZZ_RANGE = (12*3);
__attribute__((unused)) static constexpr int HEAD_SEEK_RANGE = (12*3);

__attribute__((unused)) static constexpr int SPECIAL_NOTE = (HEAD_BASE_NOTE+(HEAD_BUZZ_RANGE|HEAD_SEEK_RANGE));
__attribute__((unused)) static constexpr int SPINDLE_NOTE = (SPECIAL_NOTE+0);
__attribute__((unused)) static constexpr int SINGLE_STEP_NOTE = (SPECIAL_NOTE+2);
__attribute__((unused)) static constexpr int DISK_PUSH_NOTE = (SPECIAL_NOTE+4);
__attribute__((unused)) static constexpr int DISK_INSERT_NOTE = (SPECIAL_NOTE+5);
__attribute__((unused)) static constexpr int DISK_EJECT_NOTE = (SPECIAL_NOTE+7);
__attribute__((unused)) static constexpr int DISK_PULL_NOTE = (SPECIAL_NOTE+9);
