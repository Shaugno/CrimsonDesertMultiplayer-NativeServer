// @category CrimsonDesert.Port3663
// Load only the builder package from a recorded, one-line patched Google jar.
// Ghidra and protobuf types retain their parent-loader identity.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Program;
import ghidra.program.model.address.AddressSetView;
import ghidra.util.task.TaskMonitor;
import java.net.URL;
import java.net.URLClassLoader;
import java.io.File;
import java.io.FileOutputStream;
import com.google.security.zynamics.BinExport.BinExport2;

public class R2ExportFixed extends GhidraScript {
    public void run() throws Exception {
        String[] args = getScriptArgs();
        try (var loader = new URLClassLoader(new URL[]{new File(args[1]).toURI().toURL()},
                com.google.security.binexport.BinExport2Builder.class.getClassLoader()) {
            @Override protected synchronized Class<?> loadClass(String name, boolean resolve)
                    throws ClassNotFoundException {
                if (!name.startsWith("com.google.security.binexport."))
                    return super.loadClass(name, resolve);
                Class<?> c = findLoadedClass(name);
                if (c == null) c = findClass(name);
                if (resolve) resolveClass(c);
                return c;
            }
        }) {
            Class<?> type = loader.loadClass("com.google.security.binexport.BinExport2Builder");
            Object builder = type.getConstructor(Program.class, AddressSetView.class)
                .newInstance(currentProgram, currentProgram.getMemory());
            type.getMethod("setPrependNamespace", boolean.class).invoke(builder, true);
            var data = (BinExport2)type.getMethod("build", TaskMonitor.class).invoke(builder, monitor);
            try (var out = new FileOutputStream(args[0])) { data.writeTo(out); }
            println("R2 BinExport completed: " + args[0]);
        }
    }
}
