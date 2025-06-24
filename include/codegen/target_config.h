/**
 * Target configuration header for EMLang code generation
 */

#ifndef EMLANG_TARGET_CONFIG_H
#define EMLANG_TARGET_CONFIG_H

#include <string>
#include <vector>
#include <unordered_map>

// Disable LLVM warnings
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4624) // destructor was implicitly deleted
    #pragma warning(disable: 4244) // conversion warnings
    #pragma warning(disable: 4267) // size_t conversion warnings
#endif

#include <llvm/Target/TargetOptions.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/TargetParser/Triple.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/Target/TargetMachine.h>

// Re-enable warnings
#ifdef _MSC_VER
    #pragma warning(pop)
#endif


namespace emlang {
namespace codegen {

/**
 * @brief Target architecture types
 */
enum class Architecture {
    Unknown,
    X86,            // 32-bit x86
    X86_64,         // 64-bit x86
    ARM,            // 32-bit ARM
    ARM64,          // 64-bit ARM (AArch64)
    RISCV32,        // 32-bit RISC-V
    RISCV64,        // 64-bit RISC-V
    MIPS,           // MIPS
    MIPS64,         // MIPS64
    PowerPC,        // PowerPC
    PowerPC64,      // PowerPC64
    SPARC,          // SPARC
    SPARC64,        // SPARC64
    WebAssembly,    // WebAssembly
    NVPTX,          // NVIDIA PTX
    AMDGPU,         // AMD GPU
    HEXAGON,        // Qualcomm Hexagon
    SystemZ         // IBM SystemZ
};

/**
 * @brief Operating system types
 */
enum class OperatingSystem {
    Unknown,
    Windows,
    Linux,
    macOS,
    FreeBSD,
    NetBSD,
    OpenBSD,
    Android,
    iOS,
    WebOS,
    CUDA,
    None            // Bare metal/embedded
};

/**
 * @brief ABI (Application Binary Interface) types
 */
enum class ABI {
    Unknown,
    SystemV,        // System V ABI
    Microsoft,      // Microsoft ABI
    AAPCS,          // ARM AAPCS
    AAPCS_VFP,      // ARM AAPCS with VFP
    EABI,           // Embedded ABI
    GNU,            // GNU ABI
    MUSL,           // MUSL ABI
    Android,        // Android ABI
    iOS             // iOS ABI
};

/**
 * @brief CPU feature flags
 */
struct CPUFeatures {
    bool sse = false;
    bool sse2 = false;
    bool sse3 = false;
    bool ssse3 = false;
    bool sse4_1 = false;
    bool sse4_2 = false;
    bool avx = false;
    bool avx2 = false;
    bool avx512 = false;
    bool fma = false;
    bool aes = false;
    bool bmi = false;
    bool bmi2 = false;
    bool popcnt = false;
    bool lzcnt = false;
    bool f16c = false;
    
    // ARM features
    bool neon = false;
    bool vfp = false;
    bool crypto = false;
    bool crc = false;
    
    // Additional features
    std::vector<std::string> customFeatures;
    
    /**
     * @brief Convert to LLVM feature string
     * @return LLVM-compatible feature string
     */
    std::string toLLVMString() const;
    
    /**
     * @brief Parse from LLVM feature string
     * @param features LLVM feature string
     */
    void fromLLVMString(const std::string& features);
    
    /**
     * @brief Detect host CPU features
     * @return Host CPU features
     */
    static CPUFeatures detectHost();

private:
    /**
     * @brief Parse individual feature string
     * @param feature Feature string (e.g., "+sse", "-avx")
     */
    void parseFeature(const std::string& feature);
};

/**
 * @brief Platform-specific target configuration
 */
class TargetConfig {
public:
    /**
     * @brief Target platform information
     */
    struct PlatformInfo {
        Architecture architecture = Architecture::Unknown;
        OperatingSystem operatingSystem = OperatingSystem::Unknown;
        ABI abi = ABI::Unknown;
        std::string vendor;
        std::string environment;
        bool is64Bit = true;
        bool isLittleEndian = true;
    };
    
    /**
     * @brief Compilation settings
     */
    struct CompilationSettings {
        llvm::CodeGenOpt::Level optimizationLevel = llvm::CodeGenOpt::Default;
        llvm::Reloc::Model relocationModel = llvm::Reloc::PIC_;
        llvm::CodeModel::Model codeModel = llvm::CodeModel::Small;
        bool enablePIC = true;
        bool enablePIE = false;
        bool enableStackProtector = true;
        bool enableExceptions = true;
        bool enableRTTI = true;
        bool enableDebugInfo = false;
        bool enableProfiling = false;
        bool enableSanitizers = false;
    };
    
    /**
     * @brief Constructor with triple
     * @param triple Target triple string
     */
    explicit TargetConfig(const std::string& triple = "");
    
    /**
     * @brief Constructor with platform info
     * @param platform Platform information
     */
    explicit TargetConfig(const PlatformInfo& platform);
    
    /**
     * @brief Get target triple
     * @return Target triple string
     */
    std::string getTriple() const { return triple_; }
    
    /**
     * @brief Set target triple
     * @param triple Target triple string
     */
    void setTriple(const std::string& triple);
    
    /**
     * @brief Get platform information
     * @return Platform info
     */
    const PlatformInfo& getPlatform() const { return platform_; }
    
    /**
     * @brief Set platform information
     * @param platform Platform info
     */
    void setPlatform(const PlatformInfo& platform);

    // CPU configuration
    std::string getCPU() const { return cpu_; }
    void setCPU(const std::string& cpu) { cpu_ = cpu; }
    
    /**
     * @brief Get CPU features
     * @return CPU features
     */
    const CPUFeatures& getFeatures() const { return features_; }
    
    /**
     * @brief Set CPU features
     * @param features CPU features
     */
    void setFeatures(const CPUFeatures& features) { features_ = features; }
    
    /**
     * @brief Get compilation settings
     * @return Compilation settings
     */
    const CompilationSettings& getSettings() const { return settings_; }
    
    /**
     * @brief Set compilation settings
     * @param settings Compilation settings
     */
    void setSettings(const CompilationSettings& settings) { settings_ = settings; }
    
    /**
     * @brief Get LLVM target options
     * @return LLVM target options
     */
    llvm::TargetOptions getLLVMTargetOptions() const;
    
    /**
     * @brief Set from LLVM target options
     * @param options LLVM target options
     */
    void setFromLLVMTargetOptions(const llvm::TargetOptions& options);
    
    /**
     * @brief Check if configuration is valid
     * @return True if valid
     */
    bool isValid() const;
    
    /**
     * @brief Get configuration summary
     * @return String summary
     */
    std::string getSummary() const;
    
    /**
     * @brief Create host target configuration
     * @return Host target config
     */
    static TargetConfig createHost();
    
    /**
     * @brief Create cross-compilation target
     * @param arch Target architecture
     * @param os Target operating system
     * @param abi Target ABI
     * @return Cross-compilation target config
     */
    static TargetConfig createCross(Architecture arch, OperatingSystem os, ABI abi = ABI::Unknown);

        /**
     * @brief Create target machine for configuration
     * @param config Target configuration
     * @return Target machine or error
     */
    llvm::Expected<std::unique_ptr<llvm::TargetMachine>> createTargetMachine(const PlatformInfo& platform);
    
    /**
     * @brief Parse target configuration from string
     * @param configStr Configuration string
     * @return Parsed target config
     */
    static TargetConfig parse(const std::string& configStr);

private:
    std::string triple_;
    PlatformInfo platform_;
    std::string cpu_;
    CPUFeatures features_;
    CompilationSettings settings_;
    
    /**
     * @brief Parse triple string
     */
    void parseTriple();
    
    /**
     * @brief Update triple from platform info
     */
    void updateTriple();
    
    /**
     * @brief Detect default CPU for platform
     */
    void detectDefaultCPU();
    
    /**
     * @brief Detect default features for CPU
     */
    void detectDefaultFeatures();
};

/**
 * @brief Target configuration utilities
 */
class TargetConfigUtils {
public:
    /**
     * @brief Convert architecture to string
     * @param arch Architecture
     * @return String representation
     */
    static std::string toString(Architecture arch);
    
    /**
     * @brief Convert operating system to string
     * @param os Operating system
     * @return String representation
     */
    static std::string toString(OperatingSystem os);
    
    /**
     * @brief Convert ABI to string
     * @param abi ABI type
     * @return String representation
     */
    static std::string toString(ABI abi);
    
    /**
     * @brief Parse architecture from string
     * @param str String representation
     * @return Architecture
     */
    static Architecture parseArchitecture(const std::string& str);
    
    /**
     * @brief Parse operating system from string
     * @param str String representation
     * @return Operating system
     */
    static OperatingSystem parseOperatingSystem(const std::string& str);
    
    /**
     * @brief Parse ABI from string
     * @param str String representation
     * @return ABI type
     */
    static ABI parseABI(const std::string& str);
    
    /**
     * @brief Get supported architectures
     * @return Vector of supported architectures
     */
    static std::vector<Architecture> getSupportedArchitectures();
    
    /**
     * @brief Get supported operating systems
     * @return Vector of supported operating systems
     */
    static std::vector<OperatingSystem> getSupportedOperatingSystems();
    
    /**
     * @brief Check if architecture is 64-bit
     * @param arch Architecture
     * @return True if 64-bit
     */
    static bool is64Bit(Architecture arch);
    
    /**
     * @brief Check if architecture uses little endian
     * @param arch Architecture
     * @return True if little endian
     */
    static bool isLittleEndian(Architecture arch);
    
    /**
     * @brief Get default ABI for platform
     * @param arch Architecture
     * @param os Operating system
     * @return Default ABI
     */
    static ABI getDefaultABI(Architecture arch, OperatingSystem os);
    
    /**
     * @brief Get architecture from LLVM triple
     * @param triple LLVM triple
     * @return Architecture
     */
    static Architecture getArchFromTriple(const llvm::Triple& triple);
    
    /**
     * @brief Get operating system from LLVM triple
     * @param triple LLVM triple
     * @return Operating system
     */
    static OperatingSystem getOSFromTriple(const llvm::Triple& triple);
    
    /**
     * @brief Build target triple string
     * @param arch Architecture
     * @param vendor Vendor string
     * @param os Operating system
     * @param env Environment string
     * @return Target triple string
     */
    static std::string buildTriple(Architecture arch, const std::string& vendor,
                                  OperatingSystem os, const std::string& env = "");
};

/**
 * @brief Predefined target configurations
 */
class PredefinedTargets {
public:
    // Desktop targets
    static TargetConfig Windows_x64();
    static TargetConfig Windows_x86();
    static TargetConfig Linux_x64();
    static TargetConfig Linux_x86();
    static TargetConfig macOS_x64();
    static TargetConfig macOS_ARM64();
    
    // Mobile targets
    static TargetConfig Android_ARM64();
    static TargetConfig Android_ARM();
    static TargetConfig iOS_ARM64();
    
    // Embedded targets
    static TargetConfig ARM_Cortex_M4();
    static TargetConfig RISC_V_64();
    
    // Special targets
    static TargetConfig WebAssembly();
    static TargetConfig CUDA();
    
    /**
     * @brief Get all predefined targets
     * @return Map of name to target config
     */
    static std::unordered_map<std::string, TargetConfig> getAll();
};

/**
 * @brief Get CPU features for a specific CPU and target triple
 * @param cpu CPU name
 * @param triple Target triple
 * @return CPU features string
 */
std::string getCPUFeatures(const std::string& cpu, const std::string& triple);

} // namespace codegen
} // namespace emlang

#endif // EMLANG_TARGET_CONFIG_H
