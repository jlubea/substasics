#include "substasics/dependency_injection/kernel.h"

namespace substasics { namespace dependency_injection {

	global_kernel *global_kernel::__instance = NULL;
	substasics::platform::mutex global_kernel::__initializationMutex;

}
}