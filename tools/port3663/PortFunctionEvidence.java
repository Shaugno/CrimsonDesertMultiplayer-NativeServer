// Targeted file-only disassembly/decompilation. Does not execute game code.
// @category CrimsonDesert.Port3663
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.cmd.function.CreateFunctionCmd;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import java.io.PrintWriter;

public class PortFunctionEvidence extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 2) throw new IllegalArgumentException("output-file rva [rva ...]");
        DecompInterface decompiler = new DecompInterface();
        decompiler.openProgram(currentProgram);
        try (PrintWriter out = new PrintWriter(args[0], "UTF-8")) {
            out.println("Program=" + currentProgram.getName() + " imageBase=" + currentProgram.getImageBase());
            out.println("Targeted analysis only; no whole-program matching or live evidence; adopted=0");
            for (int i = 1; i < args.length; ++i) {
                monitor.checkCancelled();
                long rva = Long.decode(args[i]);
                Address address = currentProgram.getImageBase().add(rva);
                out.println("\nRVA=" + args[i]);
                disassemble(address);
                Function function = getFunctionAt(address);
                if (function == null) function = createFunction(address, null);
                if (function == null) {
                    out.println("BLOCKED: no function at requested address");
                    continue;
                }
                CreateFunctionCmd.fixupFunctionBody(currentProgram, function, monitor);
                out.println("Function=" + function.getName() + " body=" + function.getBody());
                int count = 0;
                for (Instruction instruction : currentProgram.getListing().getInstructions(function.getBody(), true)) {
                    if (++count > 1600) { out.println("TRUNCATED instruction limit"); break; }
                    out.println(instruction.getAddress() + " " + instruction);
                }
                DecompileResults result = decompiler.decompileFunction(function, 30, monitor);
                if (result.decompileCompleted() && result.getDecompiledFunction() != null)
                    out.println(result.getDecompiledFunction().getC());
                else out.println("BLOCKED decompilation: " + result.getErrorMessage());
                out.flush();
            }
        } finally {
            decompiler.dispose();
        }
    }
}
