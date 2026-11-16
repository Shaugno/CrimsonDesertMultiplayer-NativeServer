// @category CrimsonDesert.Port3663
import ghidra.app.script.GhidraScript;
import java.io.PrintWriter;

public class R2AnalysisSummary extends GhidraScript {
    public void run() throws Exception {
        try (PrintWriter out = new PrintWriter(getScriptArgs()[0], "UTF-8")) {
            out.println("{\"program\":\"" + currentProgram.getName() + "\",\"functions\":" +
                currentProgram.getFunctionManager().getFunctionCount() + ",\"instructions\":" +
                currentProgram.getListing().getNumInstructions() + ",\"adopted\":false}");
        }
        println("R2 whole-program analysis completed; functions=" + currentProgram.getFunctionManager().getFunctionCount());
    }
}
