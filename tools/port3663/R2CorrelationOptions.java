// @category CrimsonDesert.Port3663
// Whole-image analysis for matching. Preserve explicit instruction/data refs,
// RTTI and switch analysis; avoid speculative scalar-to-code expansion through
// Denuvo constants. Defaults are retained in separate projects for comparison.
import ghidra.app.script.GhidraScript;
import java.io.PrintWriter;

public class R2CorrelationOptions extends GhidraScript {
    public void run() throws Exception {
        var options = getCurrentAnalysisOptionsAndValues(currentProgram);
        for (String name : new String[] {"Scalar Operand References", "Stack", "Decompiler Parameter ID"}) {
            if (!options.containsKey(name)) throw new IllegalStateException("Unknown analyzer " + name);
            setAnalysisOption(currentProgram, name, "false");
        }
        if (!options.containsKey("Reference.Subroutine References"))
            throw new IllegalStateException("Missing subroutine reference option");
        setAnalysisOption(currentProgram, "Reference.Subroutine References", "false");
        if (getScriptArgs().length > 1 && getScriptArgs()[1].equals("graph")) {
            setAnalysisOption(currentProgram, "x86 Constant Reference Analyzer", "false");
        }
        try (var out = new PrintWriter(getScriptArgs()[0], "UTF-8")) {
            for (var row : getCurrentAnalysisOptionsAndValues(currentProgram).entrySet())
                out.println(row.getKey() + "=" + row.getValue());
        }
        println("R2 correlation profile: whole image; no time limit; options recorded");
    }
}
