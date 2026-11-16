// @category CrimsonDesert.Port3663
import ghidra.app.script.GhidraScript;
public class R2FixtureOrphan extends GhidraScript {
    public void run() throws Exception {
        var symbols = currentProgram.getSymbolTable().getSymbols("fixture_orphan");
        if (!symbols.hasNext()) throw new Exception("Missing fixture_orphan export");
        var address = symbols.next().getAddress();
        var manager = currentProgram.getFunctionManager();
        if (manager.getFunctionAt(address) == null || getInstructionAt(address) == null)
            throw new Exception("Fixture must have an analyzed function with instructions");
        if (!manager.removeFunction(address)) throw new Exception("Cannot remove fixture function");
        println("R2 fixture removed function, retained instructions at " + address);
    }
}
