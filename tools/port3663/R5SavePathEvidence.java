// Read-only, file-only import/string callers. No game execution or new RVAs.
// @category CrimsonDesert.Port3663
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.DataIterator;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.Symbol;
import ghidra.program.model.symbol.SymbolIterator;
import java.io.PrintWriter;
import java.util.LinkedHashSet;
import java.util.HashSet;
import java.util.Set;

public class R5SavePathEvidence extends GhidraScript {
    private Set<Function> callers=new LinkedHashSet<>();
    private Set<Address> seen=new HashSet<>();
    private void refs(PrintWriter out,Address target,int depth) throws Exception {
        if(depth>3 || !seen.add(target))return;
        for(Reference ref:currentProgram.getReferenceManager().getReferencesTo(target)){
            monitor.checkCancelled();
            Address from=ref.getFromAddress();Function f=getFunctionContaining(from);
            out.println("REF "+target+" <- "+from+" "+ref.getReferenceType()+" parent="+(f==null?"none":f.getEntryPoint()+" "+f.getName()));
            if(f!=null && !f.isExternal() && !f.isThunk())callers.add(f);
            else refs(out,f==null?from:f.getEntryPoint(),depth+1);
        }
    }
    @Override public void run() throws Exception {
        String[] args=getScriptArgs();if(args.length!=1)throw new IllegalArgumentException("output-file");
        try(PrintWriter out=new PrintWriter(args[0],"UTF-8")){
            out.println("File-only save-path discovery; imageBase="+currentProgram.getImageBase()+"; adopted=0");
            SymbolIterator symbols=currentProgram.getSymbolTable().getAllSymbols(true);
            while(symbols.hasNext()){
                Symbol s=symbols.next();String n=s.getName();
                if(n.contains("SHGetFolderPath") || n.contains("SHGetKnownFolderPath") || n.contains("GetEnvironmentVariable") || n.equals("getenv") || n.equals("_wgetenv")){
                    out.println("SYMBOL "+n+" "+s.getAddress());refs(out,s.getAddress(),0);
                }
            }
            DataIterator data=currentProgram.getListing().getDefinedData(true);
            while(data.hasNext()){
                monitor.checkCancelled();Data d=data.next();if(!d.hasStringValue())continue;
                String value=String.valueOf(d.getValue());
                if(value.contains("Pearl Abyss") || value.contains("LOCALAPPDATA") || value.contains("USERPROFILE")){
                    out.println("STRING "+d.getAddress()+" "+value);refs(out,d.getAddress(),0);
                }
            }
            DecompInterface dec=new DecompInterface();dec.openProgram(currentProgram);
            try{
                int count=0;for(Function f:callers){
                    if(++count>24){out.println("TRUNCATED caller limit");break;}
                    out.println("\nCALLER "+f.getEntryPoint()+" "+f.getName());
                    DecompileResults result=dec.decompileFunction(f,30,monitor);
                    if(result.decompileCompleted() && result.getDecompiledFunction()!=null)out.println(result.getDecompiledFunction().getC());
                    else out.println("DECOMPILATION BLOCKED "+result.getErrorMessage());
                    out.flush();
                }
            }finally{dec.dispose();}
        }
    }
}
