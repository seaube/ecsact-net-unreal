#pragma once

#include "Delegates/Delegate.h"
#include <string>

namespace EcsactNetUnreal {
	/**
	 * API to get connection string to use with the Ecsact Net runtime's
	 * `ecsact_async_start`. Intended to quickly get a connection string during
	 * development before proper player authentication is in place.
	 *
	 * NOTE: May only be used in editor builds!
	 */
	ECSACTNET_API auto GetDeveloperConnectionString(
		TDelegate<void(std::string connection_string)> OnSuccess,
		TDelegate<void(std::string error_message)> OnFailure
	) -> void;
} // namespace EcsactNetUnreal
