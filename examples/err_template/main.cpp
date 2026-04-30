#include <http/server.hpp>

#include <string_view>
#include <source_location>
#include <filesystem>

namespace fs = std::filesystem;

constexpr std::string_view kFancyErrorTemplate = R"html(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Error {0} - {1}</title>
    <style>
        /* Reset & Base Theme */
        body, html {{
            margin: 0; padding: 0; height: 100%;
            font-family: system-ui, -apple-system, sans-serif;
            background-color: #0b0f19; /* Deep space blue */
            color: #e2e8f0;
            overflow: hidden;
        }}

        /* Animated Ambient Glowing Orbs */
        .glow {{
            position: absolute;
            width: 600px; height: 600px;
            background: radial-gradient(circle, rgba(56,189,248,0.15) 0%, rgba(0,0,0,0) 70%);
            border-radius: 50%;
            top: 50%; left: 50%;
            transform: translate(-50%, -50%);
            animation: pulse 8s infinite alternate ease-in-out;
            z-index: 1;
        }}
        .glow:nth-child(2) {{
            background: radial-gradient(circle, rgba(236,72,153,0.15) 0%, rgba(0,0,0,0) 70%);
            margin-left: -150px; margin-top: -100px;
            animation-delay: -4s;
        }}

        @keyframes pulse {{
            0% {{ transform: translate(-50%, -50%) scale(1); }}
            100% {{ transform: translate(-50%, -50%) scale(1.3); }}
        }}

        /* The Glass Panel */
        .container {{
            position: relative;
            z-index: 10;
            display: flex;
            align-items: center; justify-content: center;
            height: 100%;
        }}
        .card {{
            background: rgba(255, 255, 255, 0.03);
            backdrop-filter: blur(16px);
            -webkit-backdrop-filter: blur(16px);
            border: 1px solid rgba(255, 255, 255, 0.05);
            border-radius: 24px;
            padding: 4rem 3rem;
            text-align: center;
            max-width: 450px;
            box-shadow: 0 25px 50px -12px rgba(0, 0, 0, 0.5);
        }}

        /* Premium Typography */
        h1 {{
            font-size: 8rem; margin: 0; line-height: 1;
            /* Gradient text effect */
            background: linear-gradient(135deg, #38bdf8, #ec4899);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            text-shadow: 0 10px 30px rgba(236,72,153,0.2);
        }}
        h2 {{ font-size: 1.75rem; font-weight: 600; margin: 1rem 0; color: #f8fafc; }}
        p {{ color: #94a3b8; line-height: 1.6; margin-bottom: 2rem; font-size: 1.1rem; }}
        
        /* Hover-animated Button */
        .btn {{
            display: inline-block;
            padding: 0.8rem 2rem;
            border-radius: 50px;
            background: linear-gradient(135deg, #38bdf8 0%, #3b82f6 100%);
            color: white; font-weight: bold; text-decoration: none;
            transition: transform 0.2s, box-shadow 0.2s;
        }}
        .btn:hover {{
            transform: translateY(-2px);
            box-shadow: 0 10px 20px -10px #38bdf8;
        }}
    </style>
</head>
<body>
    <div class="glow"></div>
    <div class="glow"></div>
    
    <div class="container">
        <div class="card">
            <h1>{0}</h1>
            <h2>{1}</h2>
            <a href="/" class="btn">Return to Home</a>
        </div>
    </div>
</body>
</html>
)html";

static fs::path public_dir = fs::path(std::source_location::current().file_name()).remove_filename() / "public";

int main()
{
    net::Context context;
    std::string err_template(kFancyErrorTemplate);

    http::Server srv = http::ServerBuilder()
        .set_public_dir(public_dir)
        .set_error_page_template(std::move(err_template))
        .build();

    srv.run();
}
