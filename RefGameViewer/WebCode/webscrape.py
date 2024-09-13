from datetime import datetime, timedelta
import os
import re
from bs4 import BeautifulSoup  
from playwright.sync_api import sync_playwright
from http.server import SimpleHTTPRequestHandler, HTTPServer
import threading
import time 

refCenterCredentials = {"username": "stefanpatroi@gmail.com", "password": "Stefanel15"}
cometCredentials = {"username": "spatroi", "password": "Stefi2004!"}
refCenterGames = []
cometGames = []
allGames = []

abbreviations = {
    "League2 ON (W)": "L2W", "League2 ON (M)": "L2M", "League1 ON U-20 Reserves (M)": "M20", 
    "League1 ON U-20 Reserves (W)": "W20", "League1 ON Premier (M)": "L1MP", "League1 ON Premier (W)": "L1WP",
    "League1 ON Championship (M)": "L1MC", "League1 ON Championship (W)": "L1WC", "OSA Assignments": "*",
    "PHSAL": "PHSA", "Ont Cup": "OC", "Pine Glen ": "PG", "City View Park Centre F2": "CV", "The Zanchin Automotive Soccer Centre Stadium": "SC",
    "Norton Community Park ": "NCP", "Corpus Christi": "CC", "Craig Kielburger" "Craig Kielburger SS": "CK", "Redeemer":"RDMR", "Sheridan Trafalgar Campus":"SRDN", 
    "Paramount Fine Foods Centre ":"PFC", "Mattamy Sports Park":"MTMY", "North Park Turf ": "NP", "Vaughan Grove": "VG", "Sherwood Heights": "SH",
    "Victoria Park" "Victoria Park Stadium Brampton": "VP", "Downsview Park": "DP", "Sixteen Mile Sports Complex": "16M", "Centennial College": "CENT", 
    "Bishop Reding SS":"BR"
}

def main():
    path = "C:\\Ref Games"
    delete(path)
    with sync_playwright() as p:
        browser = p.chromium.launch(headless=False, slow_mo=50)
        page1 = browser.new_page()
        refCenterGames = refCenterSchedule(page1)
        page2 = browser.new_page()
        cometGames = cometSchedule(page2)
        
        if refCenterGames:
            allGames.extend(refCenterGames)
        if cometGames:
            allGames.extend(cometGames)
        makeFiles(path)
        save2files(allGames, path)
        for file in os.listdir(path):
            filePath = os.path.join(path, file)
            reformat(filePath)

        browser.close()

    serverThread = threading.Thread(target=startHttpServer, args=(path,))
    serverThread.daemon = True
    serverThread.start()

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("Shutting down...")

def startHttpServer(directory):
    print(f"Changing directory to {directory}")
    os.chdir(directory)
    handler = SimpleHTTPRequestHandler
    httpd = HTTPServer(('0.0.0.0', 8000), handler)
    print("Serving HTTP on 0.0.0.0 port 8000")
    try:
        httpd.serve_forever()
    except Exception as e:
        print(f"Error starting HTTP server: {e}")


def refCenterSchedule(page):    
    page.goto('https://www.refcentre.com/Login.aspx?Logout=true')
    page.fill('input#ctl00_ContentPlaceHolder1_txtUserID', refCenterCredentials['username'])
    page.fill('input#ctl00_ContentPlaceHolder1_txtPwd', refCenterCredentials['password'])
    page.click('input[type=submit]')
    page.hover('li.rmItem.rmFirst a.rmLink.rmRootLink')
    page.click('a.rmLink[href="ref_games_schedule.aspx"]')

    page.wait_for_selector('#ctl00_ContentPlaceHolder1_ddView')

    html = page.inner_html('#wrapper')
    soup = BeautifulSoup(html, 'html.parser')
    games = []
    isGames = True

    table = soup.find('table', {'id': 'ctl00_ContentPlaceHolder1_dgSchedule'})
    if not table:
        print("No games on Ref Center")
        isGames = False
    else:
        print("Ref Center Schedule found!")

    current_year = datetime.now().year

    if isGames == True:
        for row in table.find_all('tr')[1:]:  
            columns = row.find_all('td')
            if len(columns) < 10:
                continue

            dateText = columns[2].text.strip()
            dateTextFull = f"{dateText} {current_year}"
            try:
               gameDate = datetime.strptime(dateTextFull, '%a, %b %d %Y')
               formattedDate = gameDate.strftime('%d-%m-%y')
            except ValueError:
                 print(f"Skipping invalid date: {dateText}")
                 continue

            gameTime = columns[3].text.strip()
            location = columns[4].text.strip()   
            league = columns[5].text.strip()           
            comp = columns[7].text.strip()
    
            games.append((formattedDate, gameTime, location, league))

    

    page.hover('li.rmItem.rmFirst a.rmLink.rmRootLink')
    page.click('a.rmLink[href="ref_games_offers.aspx"]')
    html2 = page.inner_html('#secondary')
    soup2 = BeautifulSoup(html2, 'html.parser')
    isOffers = True

    tableOffers = soup2.find('table', {'id': 'ctl00_ContentPlaceHolder1_tblOffers'})
    if not tableOffers:
        print("No pending offers")
        isOffers = False
    else:
        print("Ref Center Offers Table found!")

    if isOffers == True:
        for rows in tableOffers.find_all('tr')[1:]:
            cols = rows.find_all('td')
            dateText2 = cols[2].text.strip()
            dateTextFull2 = f"{dateText2} {current_year}"
            try:
                gameDate2 = datetime.strptime(dateTextFull2, '%b %d %Y')
                formattedDate2 = gameDate2.strftime('%d-%m-%y')
            except ValueError:
                print(f"Skipping invalid date: {dateText2}")
                continue

            gameTime2 = cols[3].text.strip()
            location2 = cols[4].text.strip()   
            league2 = cols[5].text.strip()           
            comp2 = cols[7].text.strip()
    
            games.append(("Offer", formattedDate2, gameTime2, location2, league2))
    
    return games

def cometSchedule(page):
    games = []
    page.goto('https://comet.canadasoccer.com/')
    page.fill('input#username', cometCredentials['username'])
    page.fill('input#password', cometCredentials['password'])
    page.click('input[type=submit]')
    html = page.inner_html('#businessEntitiesView')
    soup = BeautifulSoup(html, 'html.parser')

    pattern = r'id="homeForm:j_idt\d+:nextMatchesTable"'

    match = re.search(pattern, html)

    if match:
        idValue = match.group(0).split('"')[1]
        idValue += '_data'
    else:
        print("ID not found.")
    

    table = soup.find('tbody', {'id': idValue})
    if not table:
        print('COMET game table not found')
        return []
    else:
        print("COMET table found!")
        
    rows = table.find_all('tr')

    for row in rows:
        columns = row.find_all('td')
        if len(columns) < 18:
            continue

        try:
            dateAndTimeText = columns[1].text.strip()
            formattedDatetime = dateAndTimeText.replace("Date/time", "").strip()
            gameDateAndTime = datetime.strptime(formattedDatetime, '%d.%m.%Y %H:%M')
            gameDate = gameDateAndTime.date()
            formatDate = gameDate.strftime('%d-%m-%y')
            gameTime = gameDateAndTime.time()
            formattedTime = gameTime.strftime('%H:%M')
        except (ValueError, IndexError):
            continue

        league = columns[6].text.strip()
        formattedLeague = league.replace("Competition type", "").strip()
        field = columns[10].text.strip()
        formattedField = field.replace("Stadium", "").strip()
        games.append((formatDate, formattedTime, formattedField, formattedLeague))
    return games

def makeFiles(path):
    if not os.path.exists(path):
        os.makedirs(path)
    today = datetime.today()
    daysAhead = 14
    delta = timedelta(days=daysAhead)
    for i in range(daysAhead + 1):
        current_date = today + timedelta(days=i)
        fileName = current_date.strftime('%d-%m-%y') + '.txt'
        file_path = os.path.join(path, fileName)
        with open(file_path, 'w') as file:
            file.write(f"Games for {current_date.strftime('%d-%m-%y')}:\n")

def save2files(gameList, path):
    isOffer = False
    for game in gameList:
        isOffer = False
        if game[0] == "Offer":
            date = game[1]
            isOffer = True
        else:
            date = game[0]
        file_path = os.path.join(path, date + '.txt')

        if isOffer == False:
            with open(file_path, 'a') as file:
                file.write(f"{game[1]} {game[2]} {game[3]}\n")
        else:
            with open(file_path, 'a') as file:
                file.write(f"{game[2]} {game[3]} {game[4]} (Game Offer)\n")

def reformat(file):
    with open(file, 'r') as f:
        content = f.read()
    for full, abbreviatted in abbreviations.items():
        content = content.replace(full, abbreviatted)
    with open(file, 'w') as f:
        f.write(content)

def delete(directory):
    if not os.path.exists(directory):
        print(f"Directory '{directory}' does not exist.")
        return
    
    for filename in os.listdir(directory):
        file_path = os.path.join(directory, filename)

        if os.path.isfile(file_path):
            os.remove(file_path)
        
        elif os.path.isdir(file_path):
            delete(file_path)

    os.rmdir(directory)

main()
