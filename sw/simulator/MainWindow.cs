using System;
using System.Threading;
using Gtk;

public partial class MainWindow: Gtk.Window
{
	delegate string ReadLineDelegate();
	private volatile bool _stop;
	private readonly Thread _consoleThread;
	private delegate void IOHandler(string s);
	private event IOHandler OnReceived;

	public MainWindow() : base(Gtk.WindowType.Toplevel)
	{
		_stop = false;
		Build();
		_consoleThread = new Thread(
			new ParameterizedThreadStart(o =>
			{
				while (!_stop)
				{
					if (OnReceived != null)
					{
						string s = Console.ReadLine();
						if (s != null) 
						{
							Gtk.Application.Invoke((se, a) => OnReceived(s));
						}
					}
				}
			}));
		_consoleThread.Start();
		OnReceived += MainWindow_OnReceived;
	}

	void MainWindow_OnReceived (string s)
	{
		if (s == null)
		{
			_log.Buffer.Text += "null string recieved\n";
			return;
		}

		if (s.StartsWith("LOG"))
		{
			_log.Buffer.Text += s + "\n";
		}
	}

	protected void OnDeleteEvent(object sender, DeleteEventArgs a)
	{
		Application.Quit();
		try{
			Console.OpenStandardInput().Write(new byte[] { 0xA }, 0, 1 );} catch (AddEmbeddedXid) {
		}
		_stop = true;
		a.RetVal = true;
	}
}
