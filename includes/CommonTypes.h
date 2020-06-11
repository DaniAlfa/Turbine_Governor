#ifndef COMMONTYPES_H
#define COMMONTYPES_H
#include <cstdint>
#include <functional>

enum DrvState{Running, Stopped, COMError, VarError, UnInit};

enum QuState{ OK = 0, OverRange = 1, UnderRange = 2, ComError = 3};

enum TurbType{Francis, Kaplan, Pelton};

class IOAddr{
public:
	IOAddr() : uiHeader(0), uiModule(0), uiChannel(0), uiNumBits(0){}
	std::uint8_t uiHeader;
	std::uint8_t uiModule;
	std::uint8_t uiChannel;
	std::uint8_t uiNumBits;

	bool operator==(IOAddr const& other) const { 
		return other.uiHeader == uiHeader && other.uiModule == uiModule && other.uiChannel == uiChannel && other.uiNumBits == uiNumBits;
	}
};

namespace std {
    template<> 
    struct hash<IOAddr> {
        std::size_t operator()(IOAddr const& addr) const noexcept {
            return ((((hash<uint8_t>()(addr.uiHeader) ^ (hash<uint8_t>()(addr.uiModule) << 1)) >> 1) ^ (hash<uint8_t>()(addr.uiChannel) << 1)) >> 1) 
            ^ (hash<uint8_t>()(addr.uiNumBits) << 1);
        }
    };
}


#endif //COMMONTYPES_H
