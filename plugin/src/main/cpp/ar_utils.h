#ifndef ARCORE_UTILS
#define ARCORE_UTILS

#include "include/arcore_c_api.h"

namespace arcore_plugin {
class ScopedArPose {
public:
	explicit ScopedArPose(const ArSession *session) {
		ArPose_create(session, nullptr, &pose_);
	}
	~ScopedArPose() { ArPose_destroy(pose_); }
	ArPose *GetArPose() { return pose_; }
	// Delete copy constructors.
	ScopedArPose(const ScopedArPose &) = delete;
	void operator=(const ScopedArPose &) = delete;

private:
	ArPose *pose_;
};
} // namespace arcore_plugin

#endif //ARCORE_UTILS
