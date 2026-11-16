// @category CrimsonDesert.Port3663
// Whole-image analysis for matching. Preserve explicit instruction/data refs,
// RTTI; avoid speculative scalar-to-code expansion through Denuvo constants.
// Graph keeps switch recovery; direct omits it and records that limitation.
// Defaults are retained in separate projects for comparison.
import ghidra.app.util.headless.HeadlessScript;
import java.io.PrintWriter;

public class R2CorrelationOptions extends HeadlessScript {
    public void run() throws Exception {
        setHeadlessContinuationOption(HeadlessContinuationOption.ABORT);
        var options = getCurrentAnalysisOptionsAndValues(currentProgram);
        for (String name : new String[] {"Scalar Operand References", "Stack", "Decompiler Parameter ID"}) {
            if (!options.containsKey(name)) throw new IllegalStateException("Unknown analyzer " + name);
            setAnalysisOption(currentProgram, name, "false");
        }
        if (!options.containsKey("Reference.Subroutine References"))
            throw new IllegalStateException("Missing subroutine reference option");
        setAnalysisOption(currentProgram, "Reference.Subroutine References", "false");
        if (getScriptArgs().length > 1 && !getScriptArgs()[1].equals("correlation")) {
            setAnalysisOption(currentProgram, "x86 Constant Reference Analyzer", "false");
        }
        if (getScriptArgs().length > 1 && (getScriptArgs()[1].equals("direct") || getScriptArgs()[1].equals("native")))
            setAnalysisOption(currentProgram, "Decompiler Switch Analysis", "false");
        if (getScriptArgs().length > 1 && getScriptArgs()[1].equals("native")) {
            for (String name : new String[] {"Create Address Tables", "Function ID",
                    "Reference.Create Address Tables", "Data Reference.Create Address Tables"}) {
                if (!options.containsKey(name)) throw new IllegalStateException("Unknown analyzer " + name);
                setAnalysisOption(currentProgram, name, "false");
            }
        }
        try (var out = new PrintWriter(getScriptArgs()[0], "UTF-8")) {
            for (var row : getCurrentAnalysisOptionsAndValues(currentProgram).entrySet())
                out.println(row.getKey() + "=" + row.getValue());
        }
        println("R2 correlation profile: whole image; no time limit; options recorded");
        setHeadlessContinuationOption(HeadlessContinuationOption.CONTINUE);
    }
}
