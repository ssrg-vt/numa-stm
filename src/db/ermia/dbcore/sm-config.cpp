#include <unistd.h>
#include <numa.h>
#include "../macros.h"
#include "sm-config.h"
#include "sm-log-recover-impl.h"
#include "sm-thread.h"
#include <iostream>

uint32_t config::worker_threads = 0;
int config::numa_nodes = 0;
int config::enable_gc = 0;
std::string config::tmpfs_dir("/tmpfs");
int config::enable_safesnap = 0;
int config::enable_ssi_read_only_opt = 0;
uint64_t config::ssn_read_opt_threshold = config::SSN_READ_OPT_DISABLED;
int config::log_buffer_mb = 512;
int config::log_segment_mb = 8192;
std::string config::log_dir("");
int config::null_log_device = 0;
int config::htt_is_on= 1;
uint64_t config::node_memory_gb = 12;
int config::recovery_warm_up_policy = config::WARM_UP_NONE;
sm_log_recover_impl *config::recover_functor = nullptr;
int config::phantom_prot = 0;

uint32_t config::max_threads_per_node = 0;
bool config::loading = true;

void
config::init() {
    ALWAYS_ASSERT(worker_threads);
    // We pin threads compactly, ie., socket by socket
    // Figure out how many socket we will occupy here; this determines how
    // much memory we allocate for the centralized pool per socket too.
    const long ncpus = ::sysconf(_SC_NPROCESSORS_ONLN);
    ALWAYS_ASSERT(ncpus);
    max_threads_per_node = htt_is_on ?
        ncpus / 2 / (numa_max_node() + 1): ncpus / (numa_max_node() + 1);
    numa_nodes = (worker_threads + max_threads_per_node - 1) /  max_threads_per_node;

    thread::init();
}

void config::sanity_check() {
    ALWAYS_ASSERT(recover_functor);
    ALWAYS_ASSERT(numa_nodes);
}
