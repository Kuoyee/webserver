/**
 * 向页面中追加图片
 * @param {object} item 图片数据
 */
 function $(selector) {
    return document.querySelector(selector);
}
 function appendPic(item) {
    const {
        name,
        photographer,
        desc,
        picPath
    } = item || {}

    const html = `
    <li class="gallary-item">
        <img src="./${picPath}" />
        <p class="name">${name}</p>
        <p class="photographer">by ${photographer}</p>
        <p class="desc">${desc}</p>
    </li>`;

    $('#pics').innerHTML += html;
}

// 练习代码4
async function fetchPics() {
    try {
        // TODO: 正式接口为/pic/list，暂时用/data/data-test.json测试验证
        const response = await fetch('/data/data.json', {
            method: "GET"
        });
        console.log('返回状态', response.status);
        const result = await response.json();

        result.data.forEach(item => {
            appendPic(item); // 图片数据 {name, photographer, desc, picPath}
        })
    } catch (e) {
        console.error('查询图片发生错误', e);
        alert('查询图片发生错误');
    }
}
fetchPics();