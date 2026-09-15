using System;
using System.Windows;

namespace MouseOptimizer.UI
{
    public partial class App : Application
    {
        [STAThread]
        public static void Main()
        {
            var app = new App();
            app.InitializeComponent();
            app.Run();
        }

        protected override void OnStartup(StartupEventArgs e)
        {
            base.OnStartup(e);
            
            // Initialize application services
            try
            {
                // Check admin privileges
                var isAdmin = IsRunningAsAdmin();
                if (!isAdmin)
                {
                    MessageBox.Show(
                        "MouseOptimizer requires Administrator privileges.\n\nPlease run as Administrator.",
                        "Admin Rights Required",
                        MessageBoxButton.OK,
                        MessageBoxImage.Warning);
                    Current.Shutdown(1);
                    return;
                }

                // Initialize main window
                MainWindow = new MainWindow();
                MainWindow.Show();
            }
            catch (Exception ex)
            {
                MessageBox.Show(
                    $"Failed to initialize application:\n{ex.Message}",
                    "Initialization Error",
                    MessageBoxButton.OK,
                    MessageBoxImage.Error);
                Current.Shutdown(1);
            }
        }

        private bool IsRunningAsAdmin()
        {
            var identity = System.Security.Principal.WindowsIdentity.GetCurrent();
            var principal = new System.Security.Principal.WindowsPrincipal(identity);
            return principal.IsInRole(System.Security.Principal.WindowsBuiltInRole.Administrator);
        }

        protected override void OnExit(ExitEventArgs e)
        {
            base.OnExit(e);
            // Cleanup resources
        }
    }
}
