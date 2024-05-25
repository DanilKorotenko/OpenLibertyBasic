const vscode = require('vscode');

function getProgramName()
{
	return vscode.window.showInputBox({
		placeHolder: "Please enter the name of a libreBasic file in the workspace folder",
		value: "test.bas"
		});
}

function runEditorContents(resource)
{
	let targetResource = resource;
	if (!targetResource && vscode.window.activeTextEditor) 
	{
		targetResource = vscode.window.activeTextEditor.document.uri;
	}
	
	if (targetResource) 
	{
		vscode.debug.startDebugging(undefined, 
			{
				type: 'OpenLibertyBasicDebugger',
				name: 'Run File',
				request: 'launch',
				program: targetResource.fsPath
			},
			{ noDebug: true });
	}
}

function debugEditorContents(resource)
{
	let targetResource = resource;
	if (!targetResource && vscode.window.activeTextEditor) 
	{
		targetResource = vscode.window.activeTextEditor.document.uri;
	}
	
	if (targetResource) 
	{
		vscode.debug.startDebugging(undefined, 
			{
				type: 'OpenLibertyBasicDebugger',
				name: 'Debug File',
				request: 'launch',
				program: targetResource.fsPath,
				stopOnEntry: true
			});
	}
}

function activate(context) 
{
	console.log('Congratulations, your extension "openlibertybasicdebugger" is now active!');

	// The command has been defined in the package.json file
	// Now provide the implementation of the command with  registerCommand
	// The commandId parameter must match the command field in package.json
	context.subscriptions.push(
		vscode.commands.registerCommand('openlibertybasicdebugger.getProgramName', 		getProgramName),
		vscode.commands.registerCommand('openlibertybasicdebugger.runEditorContents', 	runEditorContents),
		vscode.commands.registerCommand('openlibertybasicdebugger.debugEditorContents', debugEditorContents),
	);

}

// This method is called when your extension is deactivated
function deactivate() 
{
	
}

module.exports = 
{
	activate,
	deactivate
}
