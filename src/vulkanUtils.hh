#pragma once

#include <vector>

bool checkValidationLayerSupport(std::vector<const char *> validationLayers);

std::vector<const char *> getRequiredInstanceExtensions();
