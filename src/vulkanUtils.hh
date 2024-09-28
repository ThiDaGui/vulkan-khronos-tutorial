#pragma once

#include <vector>

bool checkValidationLayerSupport(std::vector<const char *> validationLayers);

void listRequiredInstanceExtensions(
    const std::vector<const char *> &extensions);

std::vector<const char *> getRequiredInstanceExtensions();
