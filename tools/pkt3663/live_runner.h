#pragma once
// In-process control surface for the isolated 3663 diagnostic DLL.

struct Pkt3663LiveReport;

namespace pkt3663::live {

int start(const wchar_t *log_directory);
int stop();
int mark(const char *phase);
void write_report(Pkt3663LiveReport *report);

#ifdef PKT3663_LIVE_FIXTURE
int fixture_start(const wchar_t *log_directory, void *send_entry,
                  void *recv_entry, unsigned queue_capacity,
                  unsigned output_records);
void set_records_open_hook(void (*hook)(void *), void *user);
void set_phase_hold(void (*hold)(void *), void *user);
void fail_log_flush_after(unsigned successful_flushes);
void fail_session_short_write_containing(const char *needle);
#endif

} // namespace pkt3663::live
