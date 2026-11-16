// @category CrimsonDesert.Port3663
import ghidra.app.script.GhidraScript;
public class R2FixtureCheck extends GhidraScript {
    public void run() throws Exception {
        var symbols = currentProgram.getSymbolTable().getSymbols("fixture_orphan");
        if (!symbols.hasNext()) throw new Exception("Missing fixture export after promotion");
        var address = symbols.next().getAddress();
        if (currentProgram.getFunctionManager().getFunctionAt(address) == null)
            throw new Exception("Undefined fixture function was not promoted");
        println("R2 fixture undefined function restored: " + address);
    }
}
