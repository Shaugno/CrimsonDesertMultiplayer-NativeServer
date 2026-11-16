// @category CrimsonDesert.Port3663
// Uses the official Google BinExport extension built from recorded source.
import com.google.security.binexport.BinExport2Builder;
import ghidra.app.script.GhidraScript;
import java.io.FileOutputStream;

public class R2Export extends GhidraScript {
    public void run() throws Exception {
        var builder = new BinExport2Builder(currentProgram, currentProgram.getMemory());
        builder.setPrependNamespace(true);
        // Keep absolute addresses in both outputs, not an IDA-rebased mixture.
        var data = builder.build();
        try (var out = new FileOutputStream(getScriptArgs()[0])) { data.writeTo(out); }
        println("R2 BinExport completed: " + getScriptArgs()[0]);
    }
}
