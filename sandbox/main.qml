import QtQuick 2.5
import fbx.ui.page 1.0
import fbx.ui.controller 1.0
import fbx.async 1.0 as Async
import '../widgets'
import 'program'
import 'main'
import '../singleton/service-config'
import '../services/applaunch-service.js' as ApplaunchService
import '../services/middleware-service.js' as MiddlewareService
import '../services/image-service.js' as ImageService
import '../services/local-service.js' as LocalService
import '../services/adgateway-display-service.js' as AdgatewayDisplayService
import '../services/estat-service.js' as EstatService
import '../services/user-service.js' as UserService
import '../services/analytics-service.js' as AnalyticsService
import '../node_bundle/bundle.js' as Node
import '../singleton/user/user.js' as User

Page {
    id: self

    property alias loader: loader

    property bool isAccountFolder: false
    property bool isDeepLinkAvailable: false
    property bool isDisplayProfilePopup: false
    property bool isServiceChanged: true
    property bool hidePairingPopup: false
    property bool appIsEnabled: ApplaunchService.Service.getInstance().maintenance['enable']
    property int hideTime: 300
    property string focusOn: ''
    property string currentService: '6play'
    property string popupNameTemp
    property string selfPairingUid: ''
    property var currentFolder: null
    property var openOnFolder: null
    property var service6play: ApplaunchService.Service.getInstance().homeSettings['6play']
    property var platformSettings: []
    property var deepLink: null
    property var popUpSettings: []

    /*

#### FUNCTIONS ####

    */

    function updateList() {
        platformModel.clear();
        channelSubscription.setAvailableChannels();
        var homeSettings =  ApplaunchService.Service.getInstance().homeSettings;

        for (var key in homeSettings) {
            if (homeSettings[key].state !== 'off' && homeSettings[key].subscribed) {
                if (key !== '6play') {
                    platformModel.append({'title': key, 'settings': homeSettings[key]})
                }
            }
        }

        //if launched from special link specific service might be opened at launch
        for(var i = 0; i < platformModel.count; i++) {
            if(platformModel.get(i).title === currentService){
                platformList.forceActiveFocus()
                platformList.currentIndex = i;
                break;
            }
        }
        platform.color = platformSettings.color.P1;
    }

    function loadList() {
        updateList();
        loadFolders(openOnFolder);
        changeFocus();
    }

    function loadFolders(activeFolderId) {
        var _ = Node.modules.lodash;
        var serviceConfig = ServiceConfig.services[currentService];

        if (currentService == '6play') {
            AnalyticsService.Service.getInstance().event(('free_freeboxv6_' + currentService + '_Home'), 'Page Display');
        }

        return MiddlewareService.Service.getInstance().getFolders(currentService).then(function(data) {
            foldersModel.clear();
            var index = 0;
            return LocalService.Service.getInstance().getJson('headerService').then(function(dataMenu) {

                //fill static folders
                _.forEach(dataMenu, function(staticFolder) {
                    if(staticFolder.type === 'home' && serviceConfig && !serviceConfig.has_mea){
                        return;
                    }

                    if(staticFolder.type === 'home' && staticFolder.service[currentService]){
                        staticFolder.name += ' ' + staticFolder.service[currentService];
                    }

                    foldersModel.append(staticFolder);
                });

                //fill service folders and open on service folder
                _.forEach(data, function(folder, key) {
                    foldersModel.append(folder);
                    if (activeFolderId === folder.id) {
                        index = foldersModel.count - 1
                    }
                });

                if (isAccountFolder) {
                    foldersModel.clear();
                    var resume = {
                        "id" : 0,
                        "type": "resume",
                        "name": "Reprendre",
                        "service": {},
                        "source": ""
                      };
                    var followed = {
                        "id" : 0,
                        "type": "followed",
                        "name": "Programme suivis",
                        "service": {},
                        "source": ""
                      };
                    foldersModel.append(resume);
                    foldersModel.append(followed);

                }

                //if openfolder override flag is set
                if (openOnFolder) {
                    var folderIndex = [];
                    switch (openOnFolder) {
                        case 'selections':
                            //search by folder type
                            folderIndex = _.filter(_.range(0, foldersModel.count),function(idx){
                                return foldersModel.get(idx).type === openOnFolder;
                            });
                            break;
                        default:
                            //search by folder id
                            folderIndex = _.filter(_.range(0, foldersModel.count),function(idx){
                                return foldersModel.get(idx).id === openOnFolder;
                            });
                            break;

                    }
                    if (folderIndex.length > 0) {
                        index = folderIndex[0];
                    }
                    openOnFolder = null;
                }

                //select and open folder
                foldersList.currentIndex = index
                currentFolder = foldersList.model.get(foldersList.currentIndex)

                if (UserService.Service.getInstance().displayLegalNoticeInfo) {
                    loader.item.setNoticeInfoVisibleTrue();
                }

                openFolder(index)
            })
        })
    }

    function loadFolder(folderId, subfolderLinkName, subfolderId) {
        loader.setSource('folder.qml', {
            currentService: currentService,
            folderId: folderId,
            subfolderLinkName: subfolderLinkName,
            subfolderId:  subfolderId
        })
    }

    function openFolder(index) {
        //update programs
        currentFolder = foldersList.model.get(index);
        if (currentFolder.id !== 0) { //if it's not a custom folder (live, home, selection)
            var subfolderLinkName = ''
            var subfolderId = 0

            if (currentFolder.subfolder_link_name) {
                subfolderLinkName = currentFolder.subfolder_link_name
            }

            if (currentFolder.subfolder) {
                subfolderId = currentFolder.subfolder.id
            }

            loadFolder(currentFolder.id, subfolderLinkName, subfolderId)
        } else if(currentFolder.type === 'home'){
            loader.setSource('mea.qml', { headerHeight: platform.height}) // pass header height to compensate image ratio
        } else if(currentFolder.type === 'selections') {
            showSelectionScreen()
        } else if (currentFolder.type === 'resume') {
            loader.setSource('resume.qml', {
                currentService: currentService,
            });
        } else if (currentFolder.type === 'followed') {
            loader.setSource('followed.qml', {
                currentService: currentService,
            });
        }
    }

    function adsRedirect() {
        var _ = Node.modules.lodash;
        UserService.Service.getInstance().isFirstTimeOpen = false;
        UserService.Service.getInstance().legalNoticeInfoTime = -1;

        _.forEach(deepLink.click_tracking_url, function(value) {
            EstatService.Service.getInstance().sendUrl(value);
        })
        isDisplayProfilePopup = false;

        switch (deepLink.type) {
            case 'program':
                MiddlewareService.Service.getInstance().getProgram(deepLink.id).then(function(data){
                    currentService = data.service_display.code;
                    platformSettings = getHomeSetting();
                    updateList();
                    loadFolders(null);
                    mainPage.visible = true;
                    stack.push('program.qml', {programId: deepLink.id} )
                }).fail(function (error) {
                    popupManager('preHomeClose');
                    console.error('failed to load getProgram in home.qml: ', JSON.stringify(error));
                });
                break;

            case 'folder':
                var serviceName = ''
                _.forEach(ServiceConfig.services, function(value) {
                    if (value.id === deepLink.id) {
                        serviceName = value.code
                    }
                });

                MiddlewareService.Service.getInstance().getFolderPrograms(deepLink.id).then(function(data) {
                    currentService = data[0].service_display.code;
                    platformSettings = getHomeSetting();
                    updateList();
                    // change highlight move and resize duration to 0
                    // to set focus immediately at folder position
                    foldersList.highlightMoveDuration = 0;
                    foldersList.highlightResizeDuration = 0;

                    loadFolders(deepLink.id).then(function() {
                        foldersList.forceActiveFocus();
                        // change highlight move and resize duration to previous value
                        foldersList.highlightMoveDuration = 150;
                        foldersList.highlightResizeDuration = -1;
                        mainPage.visible = true;
                    }).then(function() {
                        popupManager('preHomeClose');
                        foldersList.forceActiveFocus();
                    });
                }).fail(function (error) {
                    console.error('failed to load getFolderPrograms in home.qml: ', JSON.stringify(error));
                });
                break;

            case 'clip':
                MiddlewareService.Service.getInstance().getVideo('clip_' + deepLink.id).then(function(data) {
                    currentService = data.service_display.code;
                    platformSettings = getHomeSetting();
                    updateList();
                    loadFolders(null);
                    mainPage.visible = true;
                    AnalyticsService.Service.getInstance().event(('free_freeboxv6_' + currentService + '_PlayerPlay'), 'click', data.title);
                    stack.pushMany([ {
                            url: 'program.qml',
                            defaults: {
                                programId: data.program.id
                            }
                        },{
                            url: 'player.qml',
                            defaults: {
                                clipId: 'clip_' + deepLink.id,
                                title: data.title,
                                currentService: data.service_display.code
                            }
                        }
                    ])
                }).fail(function (error) {
                    popupManager('preHomeClose');
                    console.error('failed to load getVideo in home.qml: ', JSON.stringify(error));
                });
                break;
        }
    }

    function isKidsMode() {
        var userId = UserService.Service.getInstance().currentUserId
        var kidsId = UserService.Service.getInstance().kidsProfile.uid

        if (userId === kidsId) {
            return true;
        } else {
            return false;
        }
    }

    function openKidsMode() {
        var folderId = ApplaunchService.Service.getInstance().kids.folder_id;
        var kidsService = ApplaunchService.Service.getInstance().kids.service;
        UserService.Service.getInstance().isFirstTimeOpen = false;

        stack.replace('kids.qml', {currentService: kidsService, folderId: folderId});
    }

    function showSelectionScreen() {
        if(!UserService.Service.getInstance().isGuestMode()){
            loader.setSource("selections.qml", {
                 currentService: currentService,
                 headerHeight: platform.height
             })
        } else {
            UserService.Service.getInstance().getListPairedUsersToBox().then(function(rsp) {
                var isEmptyListPairedUsersToBox = rsp.length === 0 ? true : false
                if(isEmptyListPairedUsersToBox){
                    loader.setSource('pairing-step1.qml', {
                        headerHeight: platform.height,
                        focusUpElement: foldersList
                    })
                } else {
                    loader.setSource('select-profile.qml', {
                        headerHeight: platform.height,
                        focusUpElement: foldersList
                    })
                }
            });
        }
    }

    function backClicked() {
        if (currentService === '6play' && foldersList.model.get(0).id === currentFolder.id) {
            Qt.quit()
        } else if (foldersList.model.get(0).id === currentFolder.id) {
            currentService = '6play';
            platformSettings = service6play;
            updateList();
            loadFolders(0);
        } else {
            loadFolders(0);
        }
    }

    function delay(delay, callback){
        if (timer.running) {
            return;
        }
        timer.callback = callback;
        timer.interval = delay + 1;
        timer.running = true;
    }


    function changeFocus(focus) {
        if (focus) {
            focusOn = focus
        }

        switch (focusOn) {
            case 'profile':
                profile.forceActiveFocus();
                break;
            case 'preHomeAds':
                preHomeAds.preHomeButton.forceActiveFocus();
                break;
            case 'folders':
                foldersList.forceActiveFocus();
                break;
            case '6play':
                item6play.forceActiveFocus();
                break;
            default:
                break;
        }
    }

    function checkPairingPopup() {
        var isFirstTimePairing = UserService.Service.getInstance().isFirstTimePairing;
        var connections = UserService.Service.getInstance().defaultConnections;
        var profile = UserService.Service.getInstance().profile;
        var isAvailable = UserService.Service.getInstance().isAvailable

        if (!isFirstTimePairing) {
            return false;
        }

        if (connections < profile.displayPopinFirst) {
            return false;
        }

        if (!isAvailable) {
            return false;
        }

        return (connections - profile.displayPopinFirst) % profile.displayPopinInterval === 0;
    }

    function getTypePairingPopup() {
        var connections = UserService.Service.getInstance().defaultConnections;
        var profile = UserService.Service.getInstance().profile;
        var index = ((connections - profile.displayPopinFirst) / profile.displayPopinInterval) % profile.displayPairingSequence.length;

        return profile.displayPairingSequence[index];
    }

    function isPopupExist() {
        var _ = Node.modules.lodash;

        return (typeof stack !== 'undefined' && _.has(stack, 'popupLoader'));
    }


    function getPreHomeAds() {
        var _ = Node.modules.lodash;
        UserService.Service.getInstance().isPreHomeOpened = true;
        mainPage.visible = false
        var adsServiceName = '1';

        _.forEach(ServiceConfig.services, function(value) {
            if (value.code === currentService) {
                adsServiceName = value.id
            }
        });

        AdgatewayDisplayService.Service.getInstance().getAds('services', adsServiceName, ['pre_home','pre_home_video']).then(function(item) {
            if (item === null) {
                popupManager('preHomeCloseAndOpenHome');
            } else {
                preHomeAds.visible = true
                preHomeAds.background.source = ImageService.Service.getInstance().getImage(item.external_image_id, parseInt(self.width), parseInt(self.height));

                if (!_.isEmpty(item.deep_links)) {
                    deepLink = item.deep_links[0]
                    isDeepLinkAvailable = true
                }

                if (item.external_video_id) {
                    preHomeAds.videoId = item.external_video_id

                }

                preHomeAds.visible = true;
                focusOn = 'preHomeAds';
                preHomeAds.startAds(currentService);
                loadList()
            }
        });
    }

    function isCurrentItemComingSoon(index, settings) {
        return platformList.currentIndex === index && settings.state === 'comingSoon' && platformList.activeFocus
    }

    function closeCsaErrorPopup() {
        csaErrorPopup.visible = false;
        loader.forceActiveFocus();
    }

    function close16CsaErrorPopup() {
        csa16ErrorPopup.visible = false;
        loader.forceActiveFocus();
    }

    function initPopupSettings() {
        var _ = Node.modules.lodash;
        mainPage.visible = false
        platformSettings = getHomeSetting();

        popUpSettings.services = '';
        popUpSettings.isPreHomeOpened = UserService.Service.getInstance().isPreHomeOpened
        popUpSettings.showPreHome = ApplaunchService.Service.getInstance().adsDisplay[currentService].pre_home
        popUpSettings.isEmptyListPairedUsersToBox = true
        popUpSettings.isOnePairedUsersToBox = false
        var calls = [];

        var callMiddlewareService = MiddlewareService.Service.getInstance().getServices().then(function(data) {
            popUpSettings.services = _.map(data, function(service) {
                return ApplaunchService.Service.getInstance().override(service)
            });
        });

        var callUserService = UserService.Service.getInstance().getListPairedUsersToBox().then(function(rsp) {
            popUpSettings.isEmptyListPairedUsersToBox = rsp.length === 0 ? true : false
            popUpSettings.isOnePairedUsersToBox = rsp.length === 1 ? true : false
        });

        calls.push(callMiddlewareService);
        calls.push(callUserService);

        new Async.Deferred.List(calls).then(function() {
            ServiceConfig.services = _.keyBy(popUpSettings.services, 'code')
            popupManager('preHomeOpen');
        });
    }

    function popupManager(popupName) {
        var userService = UserService.Service.getInstance();

        switch (popupName) {
            case 'preHomeOpen':
                preHomeOpen();
                break;

            case 'preHomeClose':
                closePreHomeAdsTimer.stop();
                preHomeAds.visible = false;
                popupManager('closeAllPopup');
                break;

            case 'preHomeCloseAndOpenHome':
                closePreHomeAdsTimer.stop();
                preHomeAds.visible = false;
                popupManager('selfParing')
                break;

            case 'paring':
                paring(userService);
                break;

            case 'paringClose':
                hidePairingPopup = false;
                pairingPopup.visible = false;
                break;

            case 'paringCloseAndOpenHomeProfile':
                hidePairingPopup = false;
                pairingPopup.visible = false;
                popupManager('openNPS');
                break;

            case 'homeProfile':
                homeProfile(userService);
                break;

            case 'firstUserParing':
                firstUserParing(userService);
                break;

            case 'firstUserParingLater':
                firstUserParingLater(userService);
                break;

            case 'openNPS':
                openNPS();
                break;
            case 'selfParing':
                selfParing(userService);
                break;

            case 'openNPSQuestion':
                openNPSQuestion();
                break;

            case 'closeAllPopup':
            default:
                mainPage.visible = true;
                profile.forceActiveFocus();
                loadList();
                stack.popupLoader.hidePopup();
        }
    }

    function preHomeOpen(){
        if (!popUpSettings.isPreHomeOpened && popUpSettings.showPreHome) {
            getPreHomeAds();
        } else {
            popupManager('preHomeCloseAndOpenHome');
        }
    }

    function setUserInOpenShowProfile() {
        waitForPopupLoader('closePopup');
        isDisplayProfilePopup = false;
        UserService.Service.getInstance().isFirstTimeOpen = false;
    }

    function openHomeParingPopup() {
        if ( UserService.Service.getInstance().firstRunApp !== 'true') {
            if (!popUpSettings.isEmptyListPairedUsersToBox) {
                UserService.Service.getInstance().isFirstTimeOpen = false;
                UserService.Service.getInstance().setFirstRunApp('true');
                waitForPopupLoader('showHomeFinishPopup');
            } else {
                waitForPopupLoader('showHomeParingPopup');
            }
        } else {
            stack.popupLoader.hidePopup();
        }
    }

    function selfParing(userService) {
        var _ = Node.modules.lodash;
        var networkKey = '';
        var isUserForPairing = false;
        var paringRetryAttempts = ApplaunchService.Service.getInstance().pairing.retry;
        var displayConfirm = ApplaunchService.Service.getInstance().pairing.displayConfirm;

        _.forEach(userService.networkStatus, function(network, key) {
           if (userService.networkStatus[key].status == 'unconfirmed' && !isUserForPairing) {
               networkKey = key;
               isUserForPairing = true;
               selfPairingUid = userService.networkStatus[key].uid
           }
        });

        if (userService.firstRunApp !== 'true') {
            if (displayConfirm !== 'disabled' && isUserForPairing && userService.pairingRetry >= paringRetryAttempts && userService.pairingRetryLimit < paringRetryAttempts) {
                userService.setPairingRetry(true);
                userService.setPairingRetryLimit();

                switch (ApplaunchService.Service.getInstance().pairing.displayConfirm)  {
                    case 'screenA':
                        waitForPopupLoader('showSelfParingScreenAPopup');
                        break;
                    case 'screenB':
                        waitForPopupLoader('showSelfParingScreenBPopup');
                        break;
                }
            } else {
                userService.setPairingRetry();
                popupManager('paring');
            }
        } else {
            popupManager('paring');
        }
    }

    function paring(userService) {
        waitForPopupLoader();
        if (!popUpSettings.isPreHomeOpened && popUpSettings.isEmptyListPairedUsersToBox) {
            userService.setCountConnections(false);
            // check is x time connected
            if ( checkPairingPopup() ) {
                pairingPopup.visible = true;
                pairingPopup.button.forceActiveFocus();
                pairingPopup.variantType = getTypePairingPopup();
            } else {
                popupManager('paringCloseAndOpenHomeProfile');
            }
        } else if (!popUpSettings.isPreHomeOpened && !popUpSettings.isEmptyListPairedUsersToBox) {
            userService.setCountConnections(true);
            popupManager('paringCloseAndOpenHomeProfile');
        } else {
            popupManager('paringCloseAndOpenHomeProfile');
        }
    }

    function homeProfile(userService) {
        if (!popUpSettings.isPreHomeOpened) {
            userService.isPreHomeOpened = true;
            mainPage.visible = true
            changeFocus('profile');
            loadList();

            if (popUpSettings.isEmptyListPairedUsersToBox || popUpSettings.isOnePairedUsersToBox) {
                setUserInOpenShowProfile();
            } else {
                isDisplayProfilePopup = true;
                waitForPopupLoader('showProfile');
            }

        } else if (isDisplayProfilePopup && isPopupExist()) {
            changeFocus('profile');
            waitForPopupLoader('showProfile');
        } else {
            popupManager('closeAllPopup');
        }
    }

    function openHomeParingLaterPopup() {
        if ( UserService.Service.getInstance().firstRunApp !== 'true') {
            waitForPopupLoader('showHomeParingLaterPopup')
        } else {
            stack.popupLoader.hidePopup();
        }
    }

    function closeHomeParingLaterPopup(openNextPopup) {
        profile.forceActiveFocus();

        if (!openNextPopup)  {
            openParing();
        } else {
            stack.popupLoader.hidePopup();
        }
    }

    function firstUserParing(userService) {
        if (userService.firstRunApp !== 'true') {
            if (!popUpSettings.isEmptyListPairedUsersToBox) {
                userService.isFirstTimeOpen = false;
                userService.setFirstRunApp('true');
                waitForPopupLoader('showHomeFinishPopup');
            } else {
                waitForPopupLoader('showHomeParingPopup');
            }
        } else {
            popupManager('closeAllPopup');
        }
    }

    function firstUserParingLater(userService) {
        if ( userService.firstRunApp !== 'true') {
            waitForPopupLoader('showHomeParingLaterPopup')
        } else {
            popupManager('closeAllPopup');
        }
    }

    // open popups using this function
    // there is a problem in free when call popupLoader directly
    // the problem is only when app is starting do It only in page/home.qml
    function waitForPopupLoader(popupName) {
        if(isPopupExist()) {
            mainPage.visible = true;

            switch (popupName) {
                case 'showProfile':
                    stack.popupLoader.showProfile();
                    break;
                case 'stopSlider':
                    stack.popupLoader.stopSlider();
                    break;
                case 'showHomeParingPopup':
                    stack.popupLoader.showPopup('widgets/home-paring-popup.qml');
                    break;
                case 'showHomeParingLaterPopup':
                    stack.popupLoader.showPopup('widgets/home-paring-later-popup.qml');
                    break;
                case 'showNPSPopup':
                    stack.popupLoader.showPopup('widgets/home-nps-popup.qml');
                    break;
                case 'showSelfParingPopup':
                    stack.popupLoader.showPopup('widgets/self-paring-popup.qml');
                    break;
                case 'showNPSQuestionPopup':
                    stack.popupLoader.showPopup('widgets/home-nps-question-popup.qml');
                    break;
                case 'showSelfParingDenyPopup':
                    stack.popupLoader.showPopup('widgets/self-pairing/deny-popup.qml', {uid: selfPairingUid});
                    break;
                case 'showSelfParingMoreInfoPopup':
                    stack.popupLoader.showPopup('widgets/self-pairing/more-info-popup.qml');
                    break;
                case 'showSelfParingScreenAPopup':
                    stack.popupLoader.showPopup('widgets/self-pairing/screen-a-popup.qml', {uid: selfPairingUid});
                    break;
                case 'showSelfParingScreenBPopup':
                    stack.popupLoader.showPopup('widgets/self-pairing/screen-b-popup.qml', {uid: selfPairingUid});
                    break;
                case 'closePopup':
                    stack.popupLoader.hidePopup();
                    break;
            }

            mainPage.visible = true;
        } else {

            popupNameTemp = popupName ? popupName : '';
            mainPage.visible = false;
            delay(100, function() {
                waitForPopupLoader(popupNameTemp);
            });
        }
    }

    function getConnectionsTarget(widgetFilename) {
        return isPopupExist() && stack.popupLoader.source.toString().indexOf(widgetFilename) !== -1 ? stack.popupLoader.item : null
    }

    function getHomeSetting() {
        if (currentService ==='6play' && isAccountFolder) {
            return ApplaunchService.Service.getInstance().homeSettings['account']
        } else {
            isAccountFolder = false;
            return ApplaunchService.Service.getInstance().homeSettings[currentService]
        }
    }

    function openNPS() {
        var applaunchService = ApplaunchService.Service.getInstance();

        if (applaunchService.nps && applaunchService.nps['enabled']) {
            var userService = UserService.Service.getInstance();
            var popupReset = userService.nps.displayPopinReset;

            if (popupReset !== applaunchService.nps['displayPopinReset']) {
                userService.setNPSDefaultValue(true);
                userService.setNPSDisplayPopinReset(applaunchService.nps['displayPopinReset']);
            }

            var connection = userService.nps.connection;
            var retry = userService.nps.retry;
            var isFirstDisplay = userService.nps.isFirstDisplay;
            var score = userService.nps.score;
            var isNPSOpened = userService.isNPSOpened;

            if (retry <= applaunchService.nps['retry'] && score == -1 && !isNPSOpened) {
                userService.isNPSOpened = true;
                if (isFirstDisplay && connection >=  applaunchService.nps['displayPopinFirst']) {
                    userService.setNPSFirstDisplay(false);
                    userService.setNPSConnection(true);
                    mainPage.visible = true;
                    changeFocus('profile');
                    loadList();
                    waitForPopupLoader('showNPSPopup');
                } else if (connection >=  applaunchService.nps['displayPopinInterval'] ) {
                    userService.setNPSConnection(true);
                    mainPage.visible = true;
                    changeFocus('profile');
                    loadList();
                    waitForPopupLoader('showNPSPopup')
                } else {
                    userService.setNPSConnection();
                    popupManager('homeProfile');
                }
            } else {
                popupManager('homeProfile');
            }
        } else {
            popupManager('homeProfile');
        }
    }

    function openNPSQuestion() {
        waitForPopupLoader('showNPSQuestionPopup');
    }

    function openSelfParingScreen() {
        waitForPopupLoader('')
    }

    /*

#### VIEWS ####

    */

    ChannelSubscription {
        id:channelSubscription

        onUpdateHomeSettings: updateList();
    }

    Timer {
        id: timer
        repeat: false
        running: false
        property var callback

        onTriggered: callback()
    }

    PlayTheVideo {
        id: playTheVideo
        onClose: loader.forceActiveFocus();
    }

    PreHome {
        id: preHomeAds
        visible: false
        z:100

        Timer {
            id: closePreHomeAdsTimer
            interval: 30000
            onTriggered: popupManager('preHomeCloseAndOpenHome')
        }

        onAdsClicked: adsRedirect()
        onAccess6playClicked: {
            isDisplayProfilePopup = true;
            popupManager('preHomeCloseAndOpenHome');
        }
        onBackgroundError: {
            isDisplayProfilePopup = true;
            popupManager('preHomeCloseAndOpenHome');
        }
        onClosePreHomeAdsTimerStart: closePreHomeAdsTimer.start();
    }

    // MAIN PAGE
    Item {
        id: mainPage
        anchors.fill: parent
        visible: false

        Rectangle {
            id: platform
            height: 87
            z: 20
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }

            //User change user
            Item {
                id: changeProfile
                height: 50
                width: 64
                anchors {
                    top: parent.top
                    topMargin: 30
                    left: parent.left
                    leftMargin: 55
                }

                Image {
                    id: changeProfileIcon
                    source: '../static/img/change_profile.png'
                    height: 39
                    width: 39
                    anchors {
                        horizontalCenter: parent.horizontalCenter
                        verticalCenter: parent.verticalCenter
                    }
                }

                Rectangle {
                    id: changeProfileIconFocus
                    anchors {
                        horizontalCenter: parent.horizontalCenter
                        verticalCenter: parent.verticalCenter
                    }
                    color: 'transparent'
                    visible: changeProfile.activeFocus
                    radius: changeProfileIconFocus.height
                    height: 47
                    width: 47
                    border {
                        color: '#ffffff'
                        width: 4
                    }
                }

                Keys.onRightPressed: profile.forceActiveFocus();
                Keys.onReturnPressed: {
                    isDisplayProfilePopup = true;
                    menuBurger.showProfile();
                }

                Keys.onDownPressed: {
                    foldersList.backUp = item6play;
                    foldersList.forceActiveFocus();
                }
            }

            //User placeholder
            Item {
                id: profile
                height: 50
                width: 60
                anchors {
                    top: parent.top
                    topMargin: 30
                    left: changeProfile.right
                }

                Rectangle {
                    id: activeProfileCircle
                    width: 60
                    height: 60
                    radius: height * 0.15
                    color: isAccountFolder ? platformSettings.color.P3 : 'transparent'
                    anchors {
                        top: parent.top
                        left: parent.left
                        topMargin: -5
                    }
                }

                Rectangle {
                    id: activeProfileCircleOpacity
                    width: 60
                    height: 30
                    color: isAccountFolder ? platformSettings.color.P2 : 'transparent'
                    anchors {
                        top: parent.top
                        left: parent.left
                        topMargin: 25
                    }
                }

                Rectangle {
                    id: activeProfile
                    width: 60
                    height: 32
                    color: isAccountFolder ? platformSettings.color.P3 : 'transparent'
                    anchors {
                        left: parent.left
                        bottom: parent.bottom
                        bottomMargin: -7
                    }
                }

                MenuBurger {
                    id: menuBurger
                    isFocus: profile.activeFocus
                }


                Keys.onLeftPressed: changeProfile.forceActiveFocus();
                Keys.onRightPressed: item6play.forceActiveFocus();

                Keys.onDownPressed: {
                    foldersList.backUp = item6play;
                    foldersList.forceActiveFocus();
                }

                //on Return update folders list
                Keys.onReturnPressed: {
                    var source = loader.source.toString();

                    if (event.key === Qt.Key_Return && source.indexOf('mea.qml') !== -1) {
                        loader.item.closeLegalNoticeInfo();
                    }

                    animationLoaderHide.running = true
                    delay(hideTime, function() {
                        currentService = '6play';
                        isAccountFolder = true;
                        currentFolder = null;
                        platformSettings = getHomeSetting();
                        isServiceChanged = true;
                        platform.color = platformSettings.color.P1;
                        loadFolders(null);
                    });
                }

                //divider right to logo
                Rectangle {
                    width: 2
                    height: 36
                    color: platformSettings.color.activeFocus
                    anchors {
                        left: profile.right
                        leftMargin: 20
                        verticalCenter: parent.verticalCenter
                    }
                }
            }

            //6play logo
            Item {
                id: platform6play
                height: 55
                width: service6play.images.logoOriginalSize.width + 30
                anchors {
                    top: parent.top
                    topMargin: 30
                    left: profile.right
                    leftMargin: 20 + 18
                }

                Rectangle {
                    id: item6play
                    width: service6play.images.logoOriginalSize.width
                    height: 55
                    focus: true
                    color: 'transparent'
                    clip: true
                    Image {
                        id: logo
                        anchors {
                            top: parent.top
                            left: parent.left
                            leftMargin: 8
                            rightMargin: 8
                        }
                        width: service6play.images.logoOriginalSize.width
                        height: service6play.images.logoOriginalSize.height
                        fillMode: Image.PreserveAspectFit
                        //logo version depending on current service
                        source: {
                            var width = service6play.images.logoOriginalSize.width
                            var height = service6play.images.logoOriginalSize.height
                            var logo = getHomeSetting().images.externalKeyLogo6play
                            ImageService.Service.getInstance().getImage(logo, width, height)
                        }
                        anchors {
                            left: parent.left
                            right: parent.right
                            verticalCenter: parent.verticalCenter
                        }
                    }

                    //on Return update folders list
                    Keys.onReturnPressed: {
                        var source = loader.source.toString();
                        AnalyticsService.Service.getInstance().event(('free_freeboxv6_' + currentService + '_Home'), 'click');

                        if (event.key === Qt.Key_Return && source.indexOf('mea.qml') !== -1) {
                            loader.item.closeLegalNoticeInfo();
                        }

                        isAccountFolder = false;
                        animationLoaderHide.running = true
                        delay(hideTime, function() {
                            currentService = '6play';
                            currentFolder = null;
                            platformSettings = getHomeSetting();
                            isServiceChanged = true;
                            platform.color = platformSettings.color.P1;
                            loadFolders(null);
                        });
                    }

                    Keys.onRightPressed: platformList.forceActiveFocus();
                    Keys.onLeftPressed: profile.forceActiveFocus();

                    Keys.onDownPressed: {
                        foldersList.backUp = item6play;
                        foldersList.forceActiveFocus();
                    }

                    FocusMarker {
                        visible: item6play.activeFocus
                    }
                }

                //divider right to logo
                Rectangle {
                    width: 2
                    height: 36
                    color: platformSettings.color.activeFocus
                    anchors {
                        left: item6play.right
                        leftMargin: 20
                        verticalCenter: parent.verticalCenter
                    }
                }
            }

            Item {
                anchors {
                    top: parent.top
                    topMargin: 30
                    left: platform6play.right
                    leftMargin: 20
                    right: parent.right
                }
                height: 55
                clip: true

                //list of services logos
                ListView {
                    id: platformList
                    anchors {
                        fill: parent
                        rightMargin: 50
                    }
                    spacing: 10
                    height: parent.height
                    displayMarginEnd: 50
                    orientation: ListView.Horizontal
                    model: platformModel
                    clip: false
                    delegate: Component {
                        Item {
                            //for comingSoon services set fixed placeholder width, add +20 for side margins
                            width: settings.state === 'comingSoon' ? comingSoon.width : settings.images.logoOriginalSize.width + 20
                            height: platformList.height
                            Rectangle {
                                id: delegateItem
                                width: parent.width
                                height: 55
                                color: 'transparent'
                                clip: true
                                Image {
                                    id: logo
                                    visible: !isCurrentItemComingSoon(index, settings)
                                    width: settings.images.logoOriginalSize.width
                                    height: settings.images.logoOriginalSize.height
                                    fillMode: Image.PreserveAspectFit
                                    //set color logo if it is current service
                                    source: {
                                        var width = settings.images.logoOriginalSize.width;
                                        var height = settings.images.logoOriginalSize.height;
                                        if(title === currentService){
                                            ImageService.Service.getInstance().getImage(settings.images.externalKeyLogoColor, width, height)
                                        } else {
                                            ImageService.Service.getInstance().getImage(settings.images.externalKeyLogoWhite, width, height)
                                        }
                                    }
                                    anchors {
                                        left: parent.left
                                        right: parent.right
                                        verticalCenter: parent.verticalCenter
                                    }
                                }
                                Rectangle {
                                    id: comingSoon
                                    visible: isCurrentItemComingSoon(index, settings)
                                    width: 92
                                    height: platformList.height
                                    color: 'transparent'
                                    Text {
                                        text: 'bientôt\ndisponible'
                                        color: '#ffffff'
                                        horizontalAlignment: Text.AlignHCenter
                                        anchors {
                                            horizontalCenter: parent.horizontalCenter
                                            verticalCenter: parent.verticalCenter
                                        }
                                        font {
                                            pixelSize: 16
                                        }
                                    }
                                }
                            }
                        }
                    }
                    highlightResizeDuration: 0
                    highlightMoveDuration: 150
                    highlight: Item{
                        FocusMarker {
                            visible: platformList.activeFocus
                        }
                    }
                    Keys.onDownPressed: {
                        foldersList.backUp = platformList
                        foldersList.forceActiveFocus()
                    }

                    Keys.onReturnPressed: {
                        var source = loader.source.toString();

                        if (event.key === Qt.Key_Return && source.indexOf('mea.qml') !== -1) {
                            loader.item.closeLegalNoticeInfo();
                        }

                        //update folders list
                        if(platformList.model.get(platformList.currentIndex).settings.state !== 'comingSoon'){
                            animationLoaderHide.running = true
                            delay(hideTime, function() {
                                currentService = platformList.model.get(platformList.currentIndex).title;
                                currentFolder = null
                                platformSettings = getHomeSetting();
                                isServiceChanged = true;
                                platform.color = platformSettings.color.P1;
                                loadFolders(null);
                            });
                        }
                    }

                    Keys.onLeftPressed: {
                        if(platformList.currentIndex - 1 >= 0){
                            decrementCurrentIndex()
                        } else {
                            item6play.forceActiveFocus()
                        }
                    }

                    Keys.onRightPressed: {
                        if(platformList.currentIndex + 1 < platformList.count){
                            incrementCurrentIndex()
                        }
                    }
                }

                ListModel {
                    id: platformModel
                }
            }
        }

        //folders list
        Rectangle {
            id: folders
            x: 0
            width: self.width
            height: foldersList.count ? 50 : 0
            color: platformSettings.color.P3
            opacity: 0.8
            z: 10
            anchors {
                top: platform.bottom
            }

            Item {
                height: parent.height
                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                    leftMargin: 150
                    rightMargin: 50
                }

                ListView {
                    //where to focus on Up key
                    property var backUp;

                    id: foldersList
                    focus: true
                    height: parent.height
                    model: foldersModel
                    spacing: 15
                    orientation: ListView.Horizontal
                    anchors {
                        top: parent.top
                        left: parent.left
                        right: parent.right
                    }
                    clip: false

                    //set in px; prevents items from disappearing to quickly
                    cacheBuffer: 200
                    displayMarginBeginning: 200

                    highlight: Item{
                        FocusMarker {
                            visible: foldersList.activeFocus
                        }
                    }
                    highlightMoveDuration: 150

                    delegate: foldersDelegate
                    Keys.onUpPressed: {
                        //go back to where you came from
                        if(typeof backUp === 'undefined'){
                            backUp = item6play
                        }
                        backUp.forceActiveFocus()
                    }
                    Keys.onReturnPressed: {
                        animationLoaderHide.targets = [content]
                        animationLoader.targets =  [content]
                        animationLoaderHide.running = true
                        delay(hideTime, function() {
                            currentFolder = null
                            isServiceChanged = true;
                            openFolder(foldersList.currentIndex)
                        });
                    }
                    Keys.onDownPressed: {
                        if(loader.status === Loader.Ready){
                            loader.focus = true
                        }
                    }
                }
                Component {
                    id: foldersDelegate
                    Rectangle {
                        width: title.contentWidth + 30
                        height: 50
                        color: 'transparent'
                        Rectangle {
                            anchors{
                                fill: parent
                                margins: 4
                            }
                            //set color for current folder
                            color: {
                                var color = 'transparent'

                                //currentFolder won't be defined on Component completed but after it is downloaded from API
                                if(currentFolder !== null) {
                                    if(id !== 0){ //if it's normal folder
                                        if(currentFolder.id === id){
                                            color = platformSettings.color.P2
                                        }
                                    } else { //if it's static folder (home | live | selection)
                                        if(currentFolder.type === type) {
                                            color = platformSettings.color.P2
                                        }
                                    }
                                }
                                color
                            }
                            Text {
                                id: title
                                text: name
                                color: platformSettings.color.activeFocus
                                anchors {
                                    verticalCenter: parent.verticalCenter
                                    horizontalCenter: parent.horizontalCenter
                                }
                                font {
                                    pixelSize: 22
                                }
                            }
                        }
                    }
                }
                ListModel {
                    id: foldersModel
                }
            }
        }

        // CONTENT
        Rectangle {
            id: content
            x: 0
            width: self.width
            clip: true
            color: platformSettings.color.P3
            anchors {
                top: folders.bottom
                //make folders row overlay the content
                topMargin: -1 * folders.height
                bottom: parent.bottom
            }
            Loader {
                id: loader
                anchors {
                    fill: parent
                }
                focus: true
                onStatusChanged: {
                    if(loader.status == Loader.Ready){
                        pairingConnections.target = null

                        if (isServiceChanged) {
                            isServiceChanged = false;
                            animationLoader.running = true;
                        }

                        if (source.toString().indexOf('pairing-step1.qml') !== -1) {
                            pairingConnections.target = item
                        } else if(source.toString().indexOf('select-profile.qml') !== -1) {
                            selectProfileConnections.target = item
                        } else if (source.toString().indexOf('mea.qml') !== -1) {
                            if (popUpSettings.isEmptyListPairedUsersToBox || popUpSettings.isOnePairedUsersToBox) {
                                loader.item.fetchLegalNoticeInfo();
                            }
                            csaErrorConnections.target = item
                        }
                    }
                }
            }
        }

        NumberAnimation {
            id: animationLoaderHide
            targets: [content, folders, activeProfile, activeProfileCircle]
            property: "opacity"
            to: 0
            duration: hideTime
            easing.type: Easing.Linear
        }

        NumberAnimation {
            id: animationLoader
            targets: [content, folders, activeProfile, activeProfileCircle]
            property: "opacity"
            from: 0
            to: 1
            duration: 600
            easing.type: Easing.Linear
        }

        Pairing {
            id: pairingPopup
            visible: false
            onVisibleChanged: {
                if(pairingPopup.visible) {
                    pairingConnections.target = pairingPopup
                } else {
                    item6play.forceActiveFocus();
                    loader.item.fetchLegalNoticeInfo(true)
                }
            }
        }

        Keys.onEscapePressed: {
            isDisplayProfilePopup = false;
            backClicked()
        }

        Keys.onBackPressed: {
            isDisplayProfilePopup = false;
            backClicked()
        }
    }

    AppDisabledScreen {
        id: appDisabled
        visible: false
        logo: ApplaunchService.Service.getInstance().homeSettings['6play'].images.externalKeyLogo6play
    }

    CsaErrorPopup {
        id: csaErrorPopup

        onCloseButtonClicked: closeCsaErrorPopup()
        Keys.onEscapePressed: closeCsaErrorPopup()
        Keys.onBackPressed: closeCsaErrorPopup()
    }

    Csa16ErrorPopup {
        id: csa16ErrorPopup

        onCloseButtonClicked: close16CsaErrorPopup();
        Keys.onEscapePressed: close16CsaErrorPopup();
        Keys.onBackPressed: close16CsaErrorPopup();
    }

    /*

#### CONNECTIONS ####

    */
    Connections {
        ignoreUnknownSignals: {
            var _ = Node.modules.lodash;
            !isPopupExist() || !_.has(stack.popupLoader.item, 'maximumAccount')
        }
        target: isPopupExist() ? stack.popupLoader.item : null
        onChangeAvatar: {
            isDisplayProfilePopup = false
            menuBurger.changeAvatar();
            changeFocus('profile');
        }
        onRefreshFolder: {
            if (UserService.Service.getInstance().displayLegalNoticeInfo) {
                loader.item.setNoticeInfoVisibleTrue();
            }

            openFolder(foldersList.currentIndex)
            if(focusInsideOnExit){
                loader.forceActiveFocus();
            } else {
                if (typeof stack.popupLoader.lastFocus !== 'undefined') {
                    stack.popupLoader.lastFocus.forceActiveFocus()
                }
            }
        }
        onKidsModeClicked: {
            openKidsMode()
        }
        onAddProfileClicked: {
            isDisplayProfilePopup = true
            menuBurger.changeAvatar()
            changeFocus('profile');
        }
    }

    Connections {
        target: getConnectionsTarget('home-nps-popup.qml')
        onButtonNPSBackClicked: {
            AnalyticsService.Service.getInstance().event('free_freeboxv6_' + currentService + '_NPS-survey_NPS-later', 'click', UserService.Service.getInstance().nps.retry);
            UserService.Service.getInstance().setNPSRetry();

            popupManager('homeProfile');
        }
        onButtonNPSScoreClicked: {
            if (ApplaunchService.Service.getInstance().nps.nextScreens) {
                popupManager('openNPSQuestion');
            } else {
                popupManager('homeProfile');
            }
        }
    }

    Connections {
        target: getConnectionsTarget('home-nps-question-popup.qml')
        onButtonNPSBackClicked: popupManager('homeProfile')
        onButtonNPSScoreClicked: popupManager('homeProfile')
    }

    Connections {
        target: getConnectionsTarget('home-paring-popup.qml')
        onHomeParingAddProfileClicked: {
            UserService.Service.getInstance().isFirstTimeOpen = false;
            UserService.Service.getInstance().setFirstRunApp('true');
            UserService.Service.getInstance().isPreHomeOpened = true;
            isDisplayProfilePopup = false;
            menuBurger.changeAvatar();
            changeFocus('profile');
            popupManager('closeAllPopup');
            stack.push('pairing-step2-options.qml', {currentService: currentService, entry: 'A'});
        }
        onHomeParingLaterClicked: popupManager('firstUserParingLater');
    }

    Connections {
        target: getConnectionsTarget('home-paring-later-popup.qml')
        onHomeParingLaterClicked: {
            UserService.Service.getInstance().isFirstTimeOpen = false;
            popupManager('closeAllPopup');
        }
    }

    Connections {
        target: getConnectionsTarget('self-paring-popup.qml');
        onCloseClicked: popupManager('paringCloseAndOpenHomeProfile');
    }

    Connections {
        id: csaErrorConnections
        target: null
        onCsaError: {
            csaErrorPopup.visible = true;
            csaErrorPopup.forceActiveFocus();
        }
        onCsa16Error: {
            csa16ErrorPopup.visible = true;
            csa16ErrorPopup.forceActiveFocus();
        }
    }

    Connections {
        id: pairingConnections
        target: null
        onPairingButtonClicked: {
            loadList();
            stack.push('pairing-step2-options.qml', {currentService: currentService, entry: entryOption})
        }
    }

    Connections {
        id: selectProfileConnections
        target: null
        onSelectProfileButtonClicked: {
            waitForPopupLoader('showProfile');
        }
    }

    Connections {
        id: selfParingScreenA
        target: getConnectionsTarget('screen-a-popup.qml')
        onConfirmButtonClicked: checkOverrideUser();
        onMoreInfoButtonClicked: waitForPopupLoader('showSelfParingMoreInfoPopup');
        onDenyButtonClicked: waitForPopupLoader('showSelfParingDenyPopup');
        onLaterButtonClicked: popupManager('paringCloseAndOpenHomeProfile');
    }

    function checkOverrideUser() {
        if (User.isOverLoadFinish()) {
            menuBurger.changeAvatar();
            changeFocus('profile');
            popupManager('closeAllPopup');
        } else {
            delay(1000, function() {
                checkOverrideUser();
            })
        }
    }

    Connections {
        id: selfParingScreenB
        target: getConnectionsTarget('screen-b-popup.qml')
        onConfirmButtonClicked: {
            menuBurger.changeAvatar();
            popupManager('closeAllPopup');
            changeFocus('profile');
        }
        onMoreInfoButtonClicked: waitForPopupLoader('showSelfParingMoreInfoPopup');
    }

    Connections {
        id: selfParingDeny
        target: getConnectionsTarget('deny-popup.qml')
        onThisIsNotMyAccountButtonClicked: popupManager('paringCloseAndOpenHomeProfile');
        onChangeMyMindButtonClicked: popupManager('paringCloseAndOpenHomeProfile');
    }

    Connections {
        id: selfParingMoreInfo
        target: getConnectionsTarget('more-info-popup.qml')
        onReturnButtonClicked: {
            switch (ApplaunchService.Service.getInstance().pairing.displayConfirm)  {
                case 'screenA':
                    waitForPopupLoader('showSelfParingScreenAPopup');
                    break;
                case 'screenB':
                    waitForPopupLoader('showSelfParingScreenBPopup');
                    break;
            }
        }
    }

    /*

#### BEHAVIORS ####

    */
    Component.onCompleted: {
        var _ = Node.modules.lodash;
        mainPage.visible = false
        platformSettings = getHomeSetting();
        if (appIsEnabled) {
            initPopupSettings();


            // This is an example
            // todo: remove it before implementing tags map
            AnalyticsService.Service.getInstance().screenview('Homepage');
        } else {
            appDisabled.visible = true;
        }
    }

    onDidDisappear: {
        playTheVideo.closeResumePopup();

        if (preHomeAds.visible) {
            popupManager('preHomeClose');
        }

        if (pairingPopup.visible) {
            popupManager('paringClose');
        }
    }

    onDidAppear: {
        var source = loader.source.toString()

        if (source.indexOf('mea.qml') !== -1) {
            loader.item.sequentialAnimationSlider.restart();
        }

        self.forceActiveFocus();

        if (isDisplayProfilePopup) {
            profile.forceActiveFocus();

            waitForPopupLoader('showProfile');
        }
    }
}
