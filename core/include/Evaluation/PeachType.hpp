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
// Contains all information about Peach Type with name PeachTypeInfo::typeName;
struct PeachTypeInfo;
using PeachTypeInfoPtr = std::shared_ptr<PeachTypeInfo>;

struct PeachTypeInfo
{
    std::string typeName;
    std::vector<PeachTypeInfoPtr> ancestors;
    std::unordered_map<std::string, PeachTypeInfoPtr> thisTypeMethods;
};

// Variables type
class PeachObject
{
public:
private:
    std::any storage_;
};
} // namespace type
} // namespace peach
