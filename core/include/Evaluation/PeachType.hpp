#pragma one

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <any>

namespace peach
{
namespace type
{
// Contains all information about Peach Type with name PeachTypeInfo::name;
struct PeachTypeInfo;
using PeachTypeInfoPtr = std::shared_ptr<PeachTypeInfo>;

struct PeachTypeInfo
{
    std::string name;
    std::vector<PeachTypeInfoPtr> ancestors;
    std::unordered_map<std::string, PeachTypeInfoPtr> fields;
};

// Variables type
class PeachObject
{
public:
private:
    std::any storage_;
    PeachTypeInfoPtr typeInfo_;
};
} // namespace type
} // namespace peach
