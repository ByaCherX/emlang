/**
 * TargetConfig implementation
 */

#include "codegen/target_config.h"
#include "codegen_error.h"

#include <llvm/ADT/StringMap.h>
#include <llvm/Support/TargetSelect.h>

namespace emlang {
namespace codegen {

// CPUFeatures implementation

std::string CPUFeatures::toLLVMString() const {
    std::string features;
    
    // X86 features
    if (sse) features += ",+sse";
    if (sse2) features += ",+sse2";
    if (sse3) features += ",+sse3";
    if (ssse3) features += ",+ssse3";
    if (sse4_1) features += ",+sse4.1";
    if (sse4_2) features += ",+sse4.2";
    if (avx) features += ",+avx";
    if (avx2) features += ",+avx2";
    if (avx512) features += ",+avx512f";
    if (fma) features += ",+fma";
    if (aes) features += ",+aes";
    if (bmi) features += ",+bmi";
    if (bmi2) features += ",+bmi2";
    if (popcnt) features += ",+popcnt";
    if (lzcnt) features += ",+lzcnt";
    if (f16c) features += ",+f16c";
    
    // ARM features
    if (neon) features += ",+neon";
    if (vfp) features += ",+vfp";
    if (crypto) features += ",+crypto";
    if (crc) features += ",+crc";
    
    // Custom features
    for (const auto& feature : customFeatures) {
        if (!feature.empty()) {
            features += "," + feature;
        }
    }
    
    // Remove leading comma
    if (!features.empty() && features[0] == ',') {
        features = features.substr(1);
    }
    
    return features;
}

void CPUFeatures::fromLLVMString(const std::string& features) {
    // Reset all features
    *this = CPUFeatures{};
    
    // Parse comma-separated features
    size_t start = 0;
    size_t end = 0;
    
    while ((end = features.find(',', start)) != std::string::npos) {
        std::string feature = features.substr(start, end - start);
        parseFeature(feature);
        start = end + 1;
    }
    
    // Parse last feature
    if (start < features.length()) {
        std::string feature = features.substr(start);
        parseFeature(feature);
    }
}

CPUFeatures CPUFeatures::detectHost() {
    CPUFeatures features;
    
    // Get host CPU features
    llvm::StringMap<bool> featureMap;
    if (llvm::sys::getHostCPUFeatures(featureMap)) {
        for (const auto& feature : featureMap) {
            if (feature.getValue()) {
                std::string featureName = feature.getKey().str();
                
                // Map LLVM feature names to our feature flags
                if (featureName == "sse") features.sse = true;
                else if (featureName == "sse2") features.sse2 = true;
                else if (featureName == "sse3") features.sse3 = true;
                else if (featureName == "ssse3") features.ssse3 = true;
                else if (featureName == "sse4.1") features.sse4_1 = true;
                else if (featureName == "sse4.2") features.sse4_2 = true;
                else if (featureName == "avx") features.avx = true;
                else if (featureName == "avx2") features.avx2 = true;
                else if (featureName == "avx512f") features.avx512 = true;
                else if (featureName == "fma") features.fma = true;
                else if (featureName == "aes") features.aes = true;
                else if (featureName == "bmi") features.bmi = true;
                else if (featureName == "bmi2") features.bmi2 = true;
                else if (featureName == "popcnt") features.popcnt = true;
                else if (featureName == "lzcnt") features.lzcnt = true;
                else if (featureName == "f16c") features.f16c = true;
                else if (featureName == "neon") features.neon = true;
                else if (featureName == "vfp") features.vfp = true;
                else if (featureName == "crypto") features.crypto = true;
                else if (featureName == "crc") features.crc = true;
                else {
                    // Add unknown features to custom features
                    features.customFeatures.push_back("+" + featureName);
                }
            }
        }
    }
    
    return features;
}

void CPUFeatures::parseFeature(const std::string& feature) {
    if (feature.empty()) return;
    
    // Handle +/- prefix
    bool enable = true;
    std::string featureName = feature;
    
    if (feature[0] == '+') {
        enable = true;
        featureName = feature.substr(1);
    } else if (feature[0] == '-') {
        enable = false;
        featureName = feature.substr(1);
    }
    
    // Map feature names
    if (featureName == "sse") sse = enable;
    else if (featureName == "sse2") sse2 = enable;
    else if (featureName == "sse3") sse3 = enable;
    else if (featureName == "ssse3") ssse3 = enable;
    else if (featureName == "sse4.1") sse4_1 = enable;
    else if (featureName == "sse4.2") sse4_2 = enable;
    else if (featureName == "avx") avx = enable;
    else if (featureName == "avx2") avx2 = enable;
    else if (featureName == "avx512f") avx512 = enable;
    else if (featureName == "fma") fma = enable;
    else if (featureName == "aes") aes = enable;
    else if (featureName == "bmi") bmi = enable;
    else if (featureName == "bmi2") bmi2 = enable;
    else if (featureName == "popcnt") popcnt = enable;
    else if (featureName == "lzcnt") lzcnt = enable;
    else if (featureName == "f16c") f16c = enable;
    else if (featureName == "neon") neon = enable;
    else if (featureName == "vfp") vfp = enable;
    else if (featureName == "crypto") crypto = enable;
    else if (featureName == "crc") crc = enable;
    else {
        // Add to custom features
        customFeatures.push_back(feature);
    }
}

// TargetConfig implementation

TargetConfig::TargetConfig(const std::string& triple) : triple_(triple) {
    if (!triple_.empty()) {
        parseTriple();
        detectDefaultCPU();
        detectDefaultFeatures();
    }
}

TargetConfig::TargetConfig(const PlatformInfo& platform) : platform_(platform) {
    updateTriple();
    detectDefaultCPU();
    detectDefaultFeatures();
}

void TargetConfig::setTriple(const std::string& triple) {
    triple_ = triple;
    parseTriple();
    detectDefaultCPU();
    detectDefaultFeatures();
}

void TargetConfig::setPlatform(const PlatformInfo& platform) {
    platform_ = platform;
    updateTriple();
    detectDefaultCPU();
    detectDefaultFeatures();
}

llvm::TargetOptions TargetConfig::getLLVMTargetOptions() const {
    llvm::TargetOptions options;
    
    // Set exception handling based on platform
    switch (platform_.operatingSystem) {
        case OperatingSystem::Windows:
            options.ExceptionModel = llvm::ExceptionHandling::WinEH;
            break;
        case OperatingSystem::WebOS:
            options.ExceptionModel = llvm::ExceptionHandling::Wasm;
            break;
        default:
            options.ExceptionModel = llvm::ExceptionHandling::DwarfCFI;
            break;
    }
    
    return options;
}

void TargetConfig::setFromLLVMTargetOptions(const llvm::TargetOptions& options) {
    // Update settings based on LLVM target options
    // This is a simplified mapping
    settings_.enableExceptions = (options.ExceptionModel != llvm::ExceptionHandling::None);
}

bool TargetConfig::isValid() const {
    return !triple_.empty() && 
           platform_.architecture != Architecture::Unknown &&
           !cpu_.empty();
}

std::string TargetConfig::getSummary() const {
    std::string summary = "Target: " + triple_ + "\n";
    summary += "CPU: " + cpu_ + "\n";
    summary += "Features: " + features_.toLLVMString() + "\n";
    summary += "Architecture: " + TargetConfigUtils::toString(platform_.architecture) + "\n";
    summary += "OS: " + TargetConfigUtils::toString(platform_.operatingSystem) + "\n";
    summary += "ABI: " + TargetConfigUtils::toString(platform_.abi) + "\n";
    return summary;
}

TargetConfig TargetConfig::createHost() {
    TargetConfig config;
    config.triple_ = llvm::sys::getDefaultTargetTriple();
    config.parseTriple();
    config.cpu_ = llvm::sys::getHostCPUName().str();
    config.features_ = CPUFeatures::detectHost();
    return config;
}

TargetConfig TargetConfig::createCross(Architecture arch, OperatingSystem os, ABI abi) {
    TargetConfig config;
    config.platform_.architecture = arch;
    config.platform_.operatingSystem = os;
    config.platform_.abi = abi;
    config.platform_.is64Bit = TargetConfigUtils::is64Bit(arch);
    config.platform_.isLittleEndian = TargetConfigUtils::isLittleEndian(arch);
    config.updateTriple();
    config.detectDefaultCPU();
    config.detectDefaultFeatures();
    return config;
}

llvm::Expected<std::unique_ptr<llvm::TargetMachine>> 
TargetManager::createTargetMachine(const PlatformInfo& platform) {
    if (!initialized_) {
        return createCodegenError(CodegenErrorType::TARGET_ERROR,
                                "TargetManager not initialized");
    }
    
    // Validate configuration
    auto validateErr = validateConfig(config);
    if (validateErr) {
        return std::move(validateErr);
    }
    
    // Lookup target
    auto target = lookupTarget(config.triple);
    if (!target) {
        return target.takeError();
    }
    
    // Create target machine
    auto targetMachine = (*target)->createTargetMachine(
        config.triple,
        config.cpu,
        config.features,
        config.targetOptions,
        config.relocModel,
        config.codeModel,
        config.optLevel
    );
    
    if (!targetMachine) {
        emlang::CodegenError::CodegenError(CodegenErrorType::InternalError,
                                "Failed to create target machine for: " + config.triple);
    }
    
    return std::unique_ptr<llvm::TargetMachine>(targetMachine);
}

TargetConfig TargetConfig::parse(const std::string& configStr) {
    // Simple parsing - in real implementation this would be more sophisticated
    return TargetConfig(configStr);
}

void TargetConfig::parseTriple() {
    llvm::Triple triple(triple_);
    platform_.architecture = TargetConfigUtils::getArchFromTriple(triple);
    platform_.operatingSystem = TargetConfigUtils::getOSFromTriple(triple);
    platform_.vendor = triple.getVendorName().str();
    platform_.environment = triple.getEnvironmentName().str();
    platform_.is64Bit = triple.isArch64Bit();
    platform_.isLittleEndian = triple.isLittleEndian();
}

void TargetConfig::updateTriple() {
    triple_ = TargetConfigUtils::buildTriple(
        platform_.architecture,
        platform_.vendor,
        platform_.operatingSystem,
        platform_.environment
    );
}

void TargetConfig::detectDefaultCPU() {
    // Set default CPU based on architecture
    switch (platform_.architecture) {
        case Architecture::X86_64:
            cpu_ = "x86-64";
            break;
        case Architecture::X86:
            cpu_ = "i686";
            break;
        case Architecture::ARM64:
            cpu_ = "generic";
            break;
        case Architecture::ARM:
            cpu_ = "arm7tdmi";
            break;
        case Architecture::WebAssembly:
            cpu_ = "generic";
            break;
        default:
            cpu_ = "generic";
            break;
    }
}

void TargetConfig::detectDefaultFeatures() {
    // Set default features based on CPU and architecture
    features_ = CPUFeatures{};
    
    // For host target, detect actual features
    if (triple_ == llvm::sys::getDefaultTargetTriple()) {
        features_ = CPUFeatures::detectHost();
    }
}

/**
 * @brief Get CPU features for a specific CPU and target triple
 * @param cpu CPU name
 * @param triple Target triple
 * @return CPU features string
 */
std::string getCPUFeatures(const std::string& cpu, const std::string& triple) {
    // Create a temporary target config
    TargetConfig config(triple);
    config.setCPU(cpu);
    
    // For host target, get actual host features
    if (triple == llvm::sys::getDefaultTargetTriple()) {
        CPUFeatures hostFeatures = CPUFeatures::detectHost();
        return hostFeatures.toLLVMString();
    }
    
    // For other targets, return empty for now
    // In a real implementation, this would query LLVM for target-specific features
    return "";
}

} // namespace codegen
} // namespace emlang
