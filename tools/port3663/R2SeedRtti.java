// Labels derived independently from each file's RTTI, never assumed function pairs.
// @category CrimsonDesert.Port3663
import ghidra.app.script.GhidraScript;
import ghidra.program.model.symbol.SourceType;
import java.nio.file.Files;
import java.nio.file.Path;

public class R2SeedRtti extends GhidraScript {
    public void run() throws Exception {
        int count = 0;
        for (String line : Files.readAllLines(Path.of(getScriptArgs()[0]))) {
            if (line.isBlank() || line.startsWith("#")) continue;
            String[] fields = line.split("\t");
            String name = fields[1].replaceAll("[^A-Za-z0-9_]", "_");
            currentProgram.getSymbolTable().createLabel(currentProgram.getImageBase().add(Long.decode(fields[0])),
                name, SourceType.USER_DEFINED);
            count++;
        }
        println("R2 RTTI data labels=" + count + "; no function correspondence seeded");
    }
}
