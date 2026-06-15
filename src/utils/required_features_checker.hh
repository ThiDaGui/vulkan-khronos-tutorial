#pragma once

#include <vulkan/vulkan.hpp>

namespace vk_tutorial
{
template <typename... ChainElements>
class RequiredFeaturesChecker
{
    vk::StructureChain<ChainElements...> required_features;

public:
    explicit RequiredFeaturesChecker(ChainElements const&... elems)
        : required_features(vk::StructureChain < ChainElements...>(elems...))
                {}

private:
    [[nodiscard]] static bool check_features_internal(
        const vk::PhysicalDeviceFeatures2& required,
        const vk::PhysicalDeviceFeatures2& supported)
    {
        return
            (!required.features.samplerAnisotropy || supported.features.samplerAnisotropy);
    }

    [[nodiscard]] static bool check_features_internal(
        const vk::PhysicalDeviceVulkan11Features& required,
        const vk::PhysicalDeviceVulkan11Features& supported)
    {
        return
            (!required.shaderDrawParameters || supported.shaderDrawParameters);
    }

    [[nodiscard]] static bool check_features_internal(
        const vk::PhysicalDeviceVulkan12Features& required,
        const vk::PhysicalDeviceVulkan12Features& supported)
    {
        return
            (!required.scalarBlockLayout || supported.scalarBlockLayout) &&
            (!required.bufferDeviceAddress || supported.bufferDeviceAddress);
    }

    [[nodiscard]] static bool check_features_internal(
        const vk::PhysicalDeviceVulkan13Features& required,
        const vk::PhysicalDeviceVulkan13Features& supported)
    {
        return
            (!required.synchronization2 || supported.synchronization2) &&
            (!required.dynamicRendering || supported.dynamicRendering);
    }

    template <typename T>
    [[nodiscard]] bool check_supported(const vk::StructureChain<ChainElements...>& supported_features) const
    {
        const T required_features_struct = required_features.template get<T>();
        const T supported_features_struct = supported_features.template get<T>();

        return check_features_internal(required_features_struct, supported_features_struct);
    }

    template <typename T1, typename T2, typename... Ts>
    [[nodiscard]] bool check_supported(const vk::StructureChain<ChainElements...>& supported_features) const
    {
        return check_supported<T1>(supported_features) &&
               check_supported<T2, Ts...>(supported_features);
    }

public:
    [[nodiscard]] bool check_supported(const vk::PhysicalDevice& physical_device) const
    {
        vk::StructureChain < ChainElements
        ...
        >
        supported_features = physical_device.getFeatures2<ChainElements...>();
        return check_supported<ChainElements...>(supported_features);
    }

    template <typename T = std::tuple_element_t<0, std::tuple<ChainElements...>>, size_t Which = 0>
    [[nodiscard]] T& get() & noexcept
    {
        return required_features.template get<T, Which>();
    }

    template <typename T = std::tuple_element_t<0, std::tuple<ChainElements...>>, size_t Which = 0>
    [[nodiscard]] T const& get() const & noexcept
    {
        return required_features.template get<T, Which>();
    }

    template <typename T = std::tuple_element_t<0, std::tuple<ChainElements...>>, size_t Which = 0>
    [[nodiscard]] T&& get() && noexcept
    {
        return required_features.template get<T, Which>();
    }
};
} // vk_tutorial
