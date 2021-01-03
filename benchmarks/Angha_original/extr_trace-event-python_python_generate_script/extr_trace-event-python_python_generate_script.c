#define NULL ((void*)0)
typedef unsigned long size_t;  // Customize by platform.
typedef long intptr_t; typedef unsigned long uintptr_t;
typedef long scalar_t__;  // Either arithmetic or pointer type.
/* By default, we understand bool (as a convenience). */
typedef int bool;
#define false 0
#define true 1

/* Forward declarations */
typedef  struct TYPE_2__   TYPE_1__ ;

/* Type definitions */
struct pevent {int dummy; } ;
struct format_field {char* name; int flags; struct format_field* next; } ;
struct TYPE_2__ {struct format_field* fields; } ;
struct event_format {char* system; char* name; TYPE_1__ format; } ;
typedef  int /*<<< orphan*/  FILE ;

/* Variables and functions */
 int FIELD_IS_FLAG ; 
 int FIELD_IS_SIGNED ; 
 int FIELD_IS_STRING ; 
 int FIELD_IS_SYMBOLIC ; 
 int PATH_MAX ; 
 int /*<<< orphan*/  fclose (int /*<<< orphan*/ *) ; 
 int /*<<< orphan*/ * fopen (char*,char*) ; 
 int /*<<< orphan*/  fprintf (int /*<<< orphan*/ *,char*,...) ; 
 int /*<<< orphan*/  sprintf (char*,char*,char const*) ; 
 int /*<<< orphan*/ * stderr ; 
 struct event_format* trace_find_next_event (struct pevent*,struct event_format*) ; 

__attribute__((used)) static int python_generate_script(struct pevent *pevent, const char *outfile)
{
	struct event_format *event = NULL;
	struct format_field *f;
	char fname[PATH_MAX];
	int not_first, count;
	FILE *ofp;

	sprintf(fname, "%s.py", outfile);
	ofp = fopen(fname, "w");
	if (ofp == NULL) {
		fprintf(stderr, "couldn't open %s\n", fname);
		return -1;
	}
	fprintf(ofp, "# perf script event handlers, "
		"generated by perf script -g python\n");

	fprintf(ofp, "# Licensed under the terms of the GNU GPL"
		" License version 2\n\n");

	fprintf(ofp, "# The common_* event handler fields are the most useful "
		"fields common to\n");

	fprintf(ofp, "# all events.  They don't necessarily correspond to "
		"the 'common_*' fields\n");

	fprintf(ofp, "# in the format files.  Those fields not available as "
		"handler params can\n");

	fprintf(ofp, "# be retrieved using Python functions of the form "
		"common_*(context).\n");

	fprintf(ofp, "# See the perf-trace-python Documentation for the list "
		"of available functions.\n\n");

	fprintf(ofp, "import os\n");
	fprintf(ofp, "import sys\n\n");

	fprintf(ofp, "sys.path.append(os.environ['PERF_EXEC_PATH'] + \\\n");
	fprintf(ofp, "\t'/scripts/python/Perf-Trace-Util/lib/Perf/Trace')\n");
	fprintf(ofp, "\nfrom perf_trace_context import *\n");
	fprintf(ofp, "from Core import *\n\n\n");

	fprintf(ofp, "def trace_begin():\n");
	fprintf(ofp, "\tprint \"in trace_begin\"\n\n");

	fprintf(ofp, "def trace_end():\n");
	fprintf(ofp, "\tprint \"in trace_end\"\n\n");

	while ((event = trace_find_next_event(pevent, event))) {
		fprintf(ofp, "def %s__%s(", event->system, event->name);
		fprintf(ofp, "event_name, ");
		fprintf(ofp, "context, ");
		fprintf(ofp, "common_cpu,\n");
		fprintf(ofp, "\tcommon_secs, ");
		fprintf(ofp, "common_nsecs, ");
		fprintf(ofp, "common_pid, ");
		fprintf(ofp, "common_comm,\n\t");

		not_first = 0;
		count = 0;

		for (f = event->format.fields; f; f = f->next) {
			if (not_first++)
				fprintf(ofp, ", ");
			if (++count % 5 == 0)
				fprintf(ofp, "\n\t");

			fprintf(ofp, "%s", f->name);
		}
		fprintf(ofp, "):\n");

		fprintf(ofp, "\t\tprint_header(event_name, common_cpu, "
			"common_secs, common_nsecs,\n\t\t\t"
			"common_pid, common_comm)\n\n");

		fprintf(ofp, "\t\tprint \"");

		not_first = 0;
		count = 0;

		for (f = event->format.fields; f; f = f->next) {
			if (not_first++)
				fprintf(ofp, ", ");
			if (count && count % 3 == 0) {
				fprintf(ofp, "\" \\\n\t\t\"");
			}
			count++;

			fprintf(ofp, "%s=", f->name);
			if (f->flags & FIELD_IS_STRING ||
			    f->flags & FIELD_IS_FLAG ||
			    f->flags & FIELD_IS_SYMBOLIC)
				fprintf(ofp, "%%s");
			else if (f->flags & FIELD_IS_SIGNED)
				fprintf(ofp, "%%d");
			else
				fprintf(ofp, "%%u");
		}

		fprintf(ofp, "\\n\" %% \\\n\t\t(");

		not_first = 0;
		count = 0;

		for (f = event->format.fields; f; f = f->next) {
			if (not_first++)
				fprintf(ofp, ", ");

			if (++count % 5 == 0)
				fprintf(ofp, "\n\t\t");

			if (f->flags & FIELD_IS_FLAG) {
				if ((count - 1) % 5 != 0) {
					fprintf(ofp, "\n\t\t");
					count = 4;
				}
				fprintf(ofp, "flag_str(\"");
				fprintf(ofp, "%s__%s\", ", event->system,
					event->name);
				fprintf(ofp, "\"%s\", %s)", f->name,
					f->name);
			} else if (f->flags & FIELD_IS_SYMBOLIC) {
				if ((count - 1) % 5 != 0) {
					fprintf(ofp, "\n\t\t");
					count = 4;
				}
				fprintf(ofp, "symbol_str(\"");
				fprintf(ofp, "%s__%s\", ", event->system,
					event->name);
				fprintf(ofp, "\"%s\", %s)", f->name,
					f->name);
			} else
				fprintf(ofp, "%s", f->name);
		}

		fprintf(ofp, "),\n\n");
	}

	fprintf(ofp, "def trace_unhandled(event_name, context, "
		"event_fields_dict):\n");

	fprintf(ofp, "\t\tprint ' '.join(['%%s=%%s'%%(k,str(v))"
		"for k,v in sorted(event_fields_dict.items())])\n\n");

	fprintf(ofp, "def print_header("
		"event_name, cpu, secs, nsecs, pid, comm):\n"
		"\tprint \"%%-20s %%5u %%05u.%%09u %%8u %%-20s \" %% \\\n\t"
		"(event_name, cpu, secs, nsecs, pid, comm),\n");

	fclose(ofp);

	fprintf(stderr, "generated Python script: %s\n", fname);

	return 0;
}