// Script MUST be run as Administrator


// https://docs.microsoft.com/nb-no/windows/win32/msi/return-values-of-jscript-and-vbscript-custom-actions
var MsiActionStatus = {
    None:   0, // not executed
    Ok:     1, // completed successfully
    Cancel: 2, // premature termination by user
    Abort:  3, // unrecoverable error
    Retry:  4, // suspended sequence to be resumed later
    Ignore: 5  // skip remaining actions
}

function LogMessage (msg) {
    // https://docs.microsoft.com/en-us/windows/win32/msi/session-message
    var record = Session.Installer.CreateRecord(0);
    record.StringData(0) = "CustomAction: " + msg;
    Session.Message(0x04000000, record);
}

function CustomActionData() {
    // get current directory & DLL filename from custom action property
    var custom_action_data = Session.Property("CustomActionData")
    LogMessage("CustomActionData=" + custom_action_data)
    return custom_action_data.split("|")
}

function RegasmPath() {
    var shell = new ActiveXObject("WScript.Shell")
    // Locate .Net install path as documented on https://docs.microsoft.com/en-us/dotnet/framework/migration-guide/how-to-determine-which-versions-are-installed
    var net_path = shell.RegRead("HKLM\\SOFTWARE\\Microsoft\\NET Framework Setup\\NDP\\v4\\Full\\InstallPath")
    return '"'+net_path+'regasm.exe"' // full quoted file-path
}

function Register () {
    // COM-register assembly
    var regasm = RegasmPath()
    var ca_data = CustomActionData() // current dir. & DLL filename
    var shell = new ActiveXObject("WScript.Shell")
    shell.CurrentDirectory = ca_data[0]
    var res = shell.Run(regasm+" /codebase "+ca_data[1], 0, true) // wait until finished
    LogMessage("Register res="+res)
    if (res == 0)
        return MsiActionStatus.Ok
    else
        return MsiActionStatus.Abort
}

function UnRegister () {
    // COM-unregister assembly
    var regasm = RegasmPath()
    var ca_data = CustomActionData() // current dir. & DLL filename
    var shell = new ActiveXObject("WScript.Shell")
    var prev_dir = shell.CurrentDirectory
    shell.CurrentDirectory = ca_data[0]
    var res = shell.Run(regasm+" /unregister /codebase "+ca_data[1], 0, true) // wait until finished
    shell.CurrentDirectory = prev_dir // change back to avoid locking the installation folder (fixes DEBUG: Error 2911: Could not remove the folder)
    LogMessage("UnRegister res="+res)
    if (res == 0)
        return MsiActionStatus.Ok
    else
        return MsiActionStatus.Abort
}


//WScript.Quit(Register())
//WScript.Quit(UnRegister())
